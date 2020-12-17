/*************************************************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2020 Intel Corporation.
 *
 * This software and the related documents are Intel copyrighted materials, and your use of them is governed
 * by the express license under which they were provided to you ("License"). Unless the License provides
 * otherwise, you may not use, modify, copy, publish, distribute, disclose or transmit this software or the
 * related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express or implied warranties, other
 * than those that are expressly stated in the License.
 *
 ************************************************************************************************************/

#include "qdl_debug.h"
#include "qdl_codes.h"
#include "qdl_pci.h"
#include "qdl_msg.h"
#include "qdl_t.h"
#include "qdl_i.h"
#include <linux/genetlink.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define QDL_DEV_LOCATION_SIZE                         20

#define QDL_INVALID_SOCKET                            -1
#define QDL_SOCKET_ERROR                              QDL_INVALID_SOCKET

#define QDL_CTRL_BUFF_SIZE                            200

static int qdl_socket = QDL_SOCKET_ERROR;
static int qdl_socket_count = 0;

/**
 * _qdl_get_ctrl_msg_status
 * @msg: message
 *
 * Returns QDL_SUCCESS if control message reports successful, otherwise error code.
 */
qdl_status_t _qdl_get_ctrl_msg_status(struct nlmsghdr *msg)
{
	struct nlmsgerr *error = NULL;
	qdl_status_t status = QDL_INVALID_PARAMS;

	switch(msg->nlmsg_type) {
	case NLMSG_NOOP:
		status = QDL_SUCCESS;
		break;
	case NLMSG_ERROR:
		error = (struct nlmsgerr*)_qdl_get_msg_data_addr((uint8_t*)msg);
		if(error->error != 0) {
			QDL_DEBUGLOG_FUNCTION_FAIL("Control message", error->error);
			status = QDL_RECEIVE_MSG_ERROR;
		} else {
			status = QDL_SUCCESS;
		}
		break;
	case NLMSG_DONE:
		status = QDL_SUCCESS;
		break;
	case NLMSG_OVERRUN:
		status = QDL_BUFFER_TOO_SMALL_ERROR;
		break;
	default:
		status = QDL_INVALID_PARAMS;
	}

	return status;
}

/**
 * _qdl_get_bus_name
 * @dscr: QDL descriptor
 * @buff: buffer with messages
 * @buff_size: buffer size
 *
 * Gets message address for provided device 'dscr'.
 * Returns message address if success, otherwise error code.
 */
char* _qdl_get_bus_name(qdl_dscr_t dscr, char* buffer)
{
	qdl_struct *dscr_data = (qdl_struct*)dscr;

	sprintf(buffer, "%04x:%02x:%02x.%1x", dscr_data->pci.seg, dscr_data->pci.bus, dscr_data->pci.dev, dscr_data->pci.fun);
	return buffer;
}

/**
 * _qdl_get_dev_msg
 * @dscr: QDL descriptor
 * @buff: buffer with messages
 * @buff_size: buffer size
 *
 * Gets message address for provided device 'dscr'.
 * Returns message address if success, otherwise error code.
 */
uint8_t* _qdl_get_dev_msg(qdl_dscr_t dscr, uint8_t *buff, uint32_t buff_size)
{
	char location[QDL_DEV_LOCATION_SIZE];
	qdl_struct *dscr_data = (qdl_struct*)dscr;
	uint8_t *msg = NULL;
	qdl_status_t status = QDL_INVALID_PARAMS;
	unsigned int msg_size = 0;
	unsigned int segment = 0;
	unsigned int bus = 0;
	unsigned int device = 0;
	unsigned int function = 0;

	/* Initialize */
	memset(location, '\0', QDL_DEV_LOCATION_SIZE);

	/* Find message for the device */
	msg = _qdl_get_next_msg(buff, buff_size, NULL);
	while(msg != NULL) {
		msg_size = buff + buff_size - msg;

		status = _qdl_get_string_attr(msg, msg_size, QDL_DEVLINK_ATTR_LOCATION, location,
		QDL_DEV_LOCATION_SIZE);
		if(status == QDL_SUCCESS) {
			sscanf(location, "%04X:%02X:%02X.%1X", &segment, &bus, &device, &function);
			if(segment == dscr_data->pci.seg && bus == dscr_data->pci.bus &&
			   device  == dscr_data->pci.dev && function == dscr_data->pci.fun) {
				return msg;
			}
		}
		msg = _qdl_get_next_msg(buff, buff_size, msg);
	}

	return NULL;
}

/**
 * _qdl_init_dscr
 * @dscr: QDL descriptor
 *
 * Initializes QDL descriptor setting appropriate family ID.
 * Returns QDL_SUCCESS if function succeeds, otherwise error code.
 */
qdl_status_t _qdl_init_dscr(qdl_dscr_t dscr)
{
	qdl_struct *qdl_dscr = (qdl_struct*)dscr;
	uint8_t *send_buff = NULL;
	uint8_t *rec_buff = NULL;
	uint8_t *msg = NULL;
	qdl_status_t status = QDL_SUCCESS;
	unsigned int send_buff_size = 0;
	unsigned int rec_buff_size = 0;
	unsigned int msg_size = 0;
	unsigned int read_bytes = 0;

	QDL_DEBUGLOG_ENTERING;

	/* Create message */
	send_buff = _qdl_create_generic_msg(GENL_ID_CTRL, CTRL_CMD_GETFAMILY, &send_buff_size);
	if(send_buff == NULL) {
		QDL_DEBUGLOG_FUNCTION_FAIL("_qdl_create_generic_msg", 0);
		return QDL_MEMORY_ERROR;
	}

	/* Get device information */
	status = qdl_send_msg(dscr, send_buff, send_buff_size);
	free(send_buff);
	if(status != QDL_SUCCESS) {
		QDL_DEBUGLOG_FUNCTION_FAIL("qdl_send_msg", status);
		return status;
	}

	rec_buff_size = QDL_REC_BUFF_SIZE;
	rec_buff = malloc(rec_buff_size);
	if(rec_buff == NULL) {
		QDL_DEBUGLOG_FUNCTION_FAIL("malloc", 0);
		return QDL_MEMORY_ERROR;
	}
	memset(rec_buff, 0, rec_buff_size);

	status = qdl_receive_msg(dscr, rec_buff, &rec_buff_size);
	if(status != QDL_SUCCESS) {
		QDL_DEBUGLOG_FUNCTION_FAIL("qdl_receive_msg", status);
		free(rec_buff);
		return status;
	}

	msg = _qdl_get_next_msg(rec_buff, rec_buff_size, NULL);
	msg_size = rec_buff + rec_buff_size - msg;
	status = _qdl_get_uint32_attr(msg, msg_size, CTRL_ATTR_FAMILY_ID, &qdl_dscr->id);
	if(status != QDL_SUCCESS) {
		QDL_DEBUGLOG_FUNCTION_FAIL("_qdl_get_uint32_attr", status);
		free(rec_buff);
		return status;
	}
	free(rec_buff);

	/* Read PCI config space */
	read_bytes = _qdl_read_pci_config_space(dscr);
	if(read_bytes == 0) {
		QDL_DEBUGLOG_FUNCTION_FAIL("_qdl_read_pci_config_space", read_bytes);
		return QDL_NO_PCI_RESOURCES;
	}

	/* Get net interface name */
	status = _qdl_get_pci_net_interface(dscr, qdl_dscr->net_interface, QDL_DRIVER_NET_INTERFACE_LENGTH);
	if(status != QDL_SUCCESS) {
		QDL_DEBUGLOG_FUNCTION_FAIL("_qdl_get_pci_net_interface", status);
		return status;
	}

	return QDL_SUCCESS;
}

/**
 * _qdl_is_devlink_supported
 * @dscr: QDL descriptor
 * @segment: PCIe segment of the device to initialize
 * @bus: PCIe bus of the device to initialize
 * @support: true if devices is supported, otherwise false
 *
 * Checks if Devlink is supported for device with provided PCI location.
 * Returns QDL_SUCCESS if function failed, otherwise error code.
 */
qdl_status_t _qdl_is_dev_supported(qdl_dscr_t dscr, bool *support)
{
	uint8_t *send_buff = NULL;
	uint8_t *rec_buff = NULL;
	uint8_t *msg = NULL;
	qdl_status_t status = QDL_SUCCESS;
	unsigned int rec_buff_size = 0;
	unsigned int send_buff_size = 0;

	QDL_DEBUGLOG_ENTERING;

	/* Get list of supported devices */
	send_buff = qdl_create_msg(dscr, QDL_CMD_GET, &send_buff_size, NULL);
	if(send_buff == NULL) {
		QDL_DEBUGLOG_FUNCTION_FAIL("qdl_create_msg", 0);
		return QDL_MEMORY_ERROR;
	}

	status = qdl_send_msg(dscr, send_buff, send_buff_size);
	free(send_buff);
	if(status != QDL_SUCCESS) {
		QDL_DEBUGLOG_FUNCTION_FAIL("qdl_send_msg", status);
		return status;
	}

	rec_buff_size = QDL_REC_BUFF_SIZE;
	rec_buff = malloc(rec_buff_size);
	if(rec_buff == NULL) {
		QDL_DEBUGLOG_FUNCTION_FAIL("malloc", status);
		return QDL_MEMORY_ERROR;
	}
	memset(rec_buff, 0, rec_buff_size);

	status = qdl_receive_msg(dscr, rec_buff, &rec_buff_size);
	if(status != QDL_SUCCESS) {
		QDL_DEBUGLOG_FUNCTION_FAIL("qdl_receive_msg", status);
		free(rec_buff);
		return status;
	}

	/* Parse received message */
	*support = false;
	msg = _qdl_get_dev_msg(dscr, rec_buff, rec_buff_size);
	if(msg != NULL) {
		*support = true;
	}
	free(rec_buff);

	return QDL_SUCCESS;
}

/**
 * qdl_get_socket
 *
 * Gets socket descriptor for Devlink communication.
 * Returns file descriptor for open socket if the function succeeds, otherwise 0.
 */
int _qdl_get_socket(void)
{
	if(qdl_socket == QDL_INVALID_SOCKET) {
		qdl_socket = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
		if(qdl_socket != QDL_INVALID_SOCKET) {
			qdl_socket_count++;
		}
	} else {
		qdl_socket_count++;
	}

	return qdl_socket;
}

/**
 * qdl_open_socket
 * @dscr: QDL descriptor
 *
 * Opens socket for Devlink communication.
 * Returns 0 if success, otherwise an error code.
 */
int _qdl_open_socket(qdl_dscr_t qdl_dscr)
{
	qdl_struct *dscr = (qdl_struct*)qdl_dscr;
	int return_code = 0;
	int address_length = sizeof(struct sockaddr_nl);

	/* Get devlink socket */
	dscr->socket = _qdl_get_socket();
	if(dscr->socket == QDL_SOCKET_ERROR) {
		return QDL_OPEN_SOCKET_ERROR;
	}

	/* Check if socket is opened already */
	if(qdl_socket_count > 1) {
		return QDL_SUCCESS;
	}

	/* Bind */
	dscr->socket_addr.nl_family = AF_NETLINK;
	dscr->socket_addr.nl_groups = 0;
	dscr->socket_addr.nl_pid = getpid();
	return_code = bind(dscr->socket, (struct sockaddr*)&dscr->socket_addr, address_length);
	if(return_code == QDL_SOCKET_ERROR) {
		return QDL_OPEN_SOCKET_ERROR;
	}

	return_code = getsockname(dscr->socket, (struct sockaddr*)&dscr->socket_addr,
				  (socklen_t*)&address_length);
	if(return_code == QDL_SOCKET_ERROR) {
		return QDL_OPEN_SOCKET_ERROR;
	}

	return QDL_SUCCESS;
}

/*************************************************************************************************************
 * QDL API
 */

/**
 * qdl_get_string_attr
 * @dscr: QDL descriptor
 * @buff: buffer with messages
 * @buff_size: buffer size
 * @type: attribute type
 * @string: returned string for attribute of type 'type'
 * @string_size: buffer size for string
 *
 * Gets string attribute data based on argument 'type' for specified device 'dscr'.
 * Returns QDL_SUCCESS if success, otherwise error code.
 */
qdl_status_t qdl_get_string_attr(qdl_dscr_t dscr, uint8_t *buff, uint32_t buff_size, uint32_t type,
		char *string, unsigned int string_size)
{
	uint8_t *msg = NULL;
	qdl_status_t status = QDL_INVALID_PARAMS;
	unsigned int msg_size = 0;

	/* Validate input parameters */
	if(dscr == NULL || buff == NULL || buff_size == 0 || string == NULL || string_size == 0) {
		return status;
	}

	msg = _qdl_get_dev_msg(dscr, buff, buff_size);
	if(msg != NULL) {
		msg_size = buff + buff_size - msg;
		status = _qdl_get_string_attr(msg, msg_size, type, string, string_size);
	}

	return status;
}

/**
 * qdl_get_string_nested_attr
 * @dscr: QDL descriptor
 * @buff: buffer with messages
 * @buff_size: buffer size
 * @name: name of the nested attribute
 * @value: buffer for a value of the nested attribute
 * @value_size: buffer size of the value
 *
 * Gets nested attribute's value based on attribute's name for specified device 'dscr'.
 * Returns QDL_SUCCESS if success, otherwise error code.
 */
qdl_status_t qdl_get_string_nested_attr(qdl_dscr_t dscr, uint8_t *buff, uint32_t buff_size, char *name,
		char *value, int value_size)
{
	uint8_t *msg = NULL;
	qdl_status_t status = QDL_INVALID_PARAMS;
	unsigned int msg_size = 0;

	/* Validate input parameters */
	if(dscr == NULL || buff == NULL || buff_size == 0 || name == NULL || value == NULL
			|| value_size == 0) {
		return status;
	}

	msg = _qdl_get_dev_msg(dscr, buff, buff_size);
	if(msg != NULL) {
		msg_size = buff + buff_size - msg;
		status = _qdl_get_string_nested_attr(msg, msg_size, name, value, value_size);
	}

	return status;
}

/**
 * qdl_send_msg
 * @dscr: QDL descriptor
 * @msg: message to sent
 * @msg_size: message buffer size
 *
 * Sends message.
 * Returns QDL_SUCCESS if the function succeeds, otherwise an error code.
 */
qdl_status_t qdl_send_msg(qdl_dscr_t dscr, uint8_t *msg, unsigned int msg_size)
{
	int return_value = 0;
	qdl_struct *dscr_data = (qdl_struct*)dscr;
	struct sockaddr_nl socket_addr;

	QDL_DEBUGLOG_ENTERING;

	/* Validate input parameters */
	if(dscr == NULL || msg == NULL || msg_size == 0) {
		return QDL_INVALID_PARAMS;
	}

	/* Send the message */
	socket_addr.nl_family = AF_NETLINK;
	socket_addr.nl_groups = 0;
	socket_addr.nl_pid = 0;
	socket_addr.nl_pad = 0;
	return_value = sendto(dscr_data->socket, msg, msg_size, 0, (struct sockaddr*)&socket_addr,
			sizeof(socket_addr));
	if(return_value == -1 || return_value != msg_size) {
		QDL_DEBUGLOG_FUNCTION_FAIL("sendto", return_value);
		return QDL_SEND_MSG_ERROR;
	}

	return QDL_SUCCESS;
}

/**
 * qdl_receive_msg
 * @dscr: QDL descriptor
 * @rec_msg_buff: buffer for received message
 * @rec_msg_size: buffer size
 *
 * Receives message.
 * Returns QDL_SUCCESS if the function succeeds, otherwise an error code.
 */
qdl_status_t qdl_receive_msg(qdl_dscr_t dscr, uint8_t *rec_msg_buff, unsigned int *rec_msg_size)
{
	uint8_t ctrl_msg[QDL_CTRL_BUFF_SIZE] = { 0 };
	struct msghdr rec_msg;
	struct iovec iov;
	struct nlmsghdr *header = (struct nlmsghdr*)ctrl_msg;
	struct nlmsghdr *msg = NULL;
	qdl_struct *dscr_data = (qdl_struct*)dscr;
	int return_value = 0;

	QDL_DEBUGLOG_ENTERING;

	/* Validate input parameters */
	if(dscr == NULL || rec_msg_buff == NULL || rec_msg_size == NULL) {
		return QDL_INVALID_PARAMS;
	}

	/* Initialize parameters */
	memset(&rec_msg, 0, sizeof(rec_msg));
	memset(&iov, 0, sizeof(iov));

	/* Receive message */
	iov.iov_base = rec_msg_buff;
	iov.iov_len = *rec_msg_size;
	rec_msg.msg_name = &dscr_data->socket_addr;
	rec_msg.msg_namelen = sizeof(dscr_data->socket_addr);
	rec_msg.msg_iov = &iov;
	rec_msg.msg_iovlen = 1;
	rec_msg.msg_control = NULL;
	rec_msg.msg_controllen = 0;
	rec_msg.msg_flags = 0;
	return_value = recvmsg(dscr_data->socket, &rec_msg, 0);
	if(return_value == -1) {
		QDL_DEBUGLOG_FUNCTION_FAIL("recvmsg", errno);
		return QDL_RECEIVE_MSG_ERROR;
	}
	*rec_msg_size = return_value;

	/* Buffer is too small */
	if(rec_msg.msg_flags & MSG_TRUNC) {
		QDL_DEBUGLOG_FUNCTION_FAIL("msg_flags", MSG_TRUNC);
		return QDL_RECEIVE_MSG_ERROR;
	}

	if(rec_msg.msg_namelen != sizeof(struct sockaddr_nl)) {
		QDL_DEBUGLOG_FUNCTION_FAIL("msg_namelen", rec_msg.msg_namelen);
		return QDL_RECEIVE_MSG_ERROR;
	}

	/* Check if we got control message 'DONE' */
	msg = (struct nlmsghdr*)_qdl_get_next_msg(rec_msg_buff, *rec_msg_size, NULL);
	while(msg != NULL) {
		if(_qdl_is_ctrl_msg(msg)) {
			return _qdl_get_ctrl_msg_status(msg);
		}
		msg = (struct nlmsghdr*)_qdl_get_next_msg(rec_msg_buff, *rec_msg_size, (uint8_t*)msg);
	}

	/* Receive control message */
	do {
		iov.iov_base = ctrl_msg;
		iov.iov_len = QDL_CTRL_BUFF_SIZE;
		return_value = recvmsg(dscr_data->socket, &rec_msg, 0);
		if(return_value == -1) {
			QDL_DEBUGLOG_FUNCTION_FAIL("recvmsg", errno);
			return QDL_RECEIVE_MSG_ERROR;
		}

		/* Buffer is too small */
		if(rec_msg.msg_flags & MSG_TRUNC) {
			QDL_DEBUGLOG_FUNCTION_FAIL("msg_flags", MSG_TRUNC);
			return QDL_RECEIVE_MSG_ERROR;
		}

		if(rec_msg.msg_namelen != sizeof(struct sockaddr_nl)) {
			QDL_DEBUGLOG_FUNCTION_FAIL("msg_namelen", rec_msg.msg_namelen);
			return QDL_RECEIVE_MSG_ERROR;
		}
	} while(_qdl_is_ctrl_msg(header) == false);

	return _qdl_get_ctrl_msg_status(header);
}

/**
 * qdl_create_msg
 * @dscr: QDL descriptor
 * @cmd_type: command type of message
 * @msg_size: size of the created message
 * @data: optional message data dependent on cmd_type
 *
 * Allocates buffer for new message of type 'cmd_type' and fills with appropriate content.
 * Returns address to the created message if success, otherwise NULL.
 */
uint8_t* qdl_create_msg(qdl_dscr_t dscr, int cmd_type, unsigned int *msg_size, void* data)
{
	char bus_name_buff[QDL_BUS_NAME_LENGTH];
	qdl_struct *dscr_data = (qdl_struct*)dscr;
	uint8_t *msg = NULL;

	QDL_DEBUGLOG_ENTERING;

	/* Validate input parameters */
	if(dscr == NULL || msg_size == NULL) {
		return NULL;
	}

	/* Allocate message buffer */
	*msg_size = _qdl_get_msg_size(cmd_type);
	if(*msg_size == 0) {
		QDL_DEBUGLOG_FUNCTION_FAIL("_qdl_get_msg_size", (int)*msg_size);
		return NULL;
	}
	msg = malloc(*msg_size);
	if(msg == NULL) {
		QDL_DEBUGLOG_FUNCTION_FAIL("malloc", 0);
		return NULL;
	}
	memset(msg, 0, *msg_size);

	/* Fill the message with the content */
	switch(cmd_type) {
	case QDL_CMD_GET:
		_qdl_put_msg_header(msg, dscr_data->id, NLM_F_REQUEST | NLM_F_ACK);
		_qdl_put_msg_extra_header(msg, cmd_type, 1);
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_BUS_NAME, "pci");
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_LOCATION, _qdl_get_bus_name(dscr, bus_name_buff));
		break;
	case QDL_CMD_PORT_GET:
	case QDL_CMD_INFO_GET:
		_qdl_put_msg_header(msg, dscr_data->id, NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP);
		_qdl_put_msg_extra_header(msg, cmd_type, 1);
		break;
	case QDL_CMD_FLASH_UPDATE:
		memset(bus_name_buff, 0, sizeof(bus_name_buff));
		_qdl_put_msg_header(msg, dscr_data->id, NLM_F_REQUEST | NLM_F_ACK);
		_qdl_put_msg_extra_header(msg, cmd_type, 1);
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_BUS_NAME, "pci");
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_LOCATION, _qdl_get_bus_name(dscr, bus_name_buff));
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_FLASH_UPDATE_FILE_NAME, (char*)data);
		break;
	default:
		free(msg);
		msg = NULL;
		*msg_size = 0;
	}

	return msg;
}

/**
 * qdl_release_dev
 * @qdl_dscr: QDL descriptor
 *
 * Releases Devlink resources and closes socket.
 */
void qdl_release_dev(qdl_dscr_t dscr)
{
	qdl_struct *dev_dscr = (qdl_struct*)dscr;

	QDL_DEBUGLOG_ENTERING;

	if(dev_dscr != NULL) {
		dev_dscr->socket = QDL_INVALID_SOCKET;
		qdl_socket_count--;
		if(qdl_socket_count == 0) {
			close(qdl_socket);
			qdl_socket = QDL_SOCKET_ERROR;
		}
		free(dev_dscr);
	}
}

/**
 * qdl_init_dev
 * @segment: PCIe segment of the device to initialize
 * @bus: PCIe bus of the device to initialize
 *
 * Initializes Devlink resources and opens socket to communicate with base driver.
 * Returns address to the QDL descriptor if the function succeeds, otherwise an error code.
 */
qdl_dscr_t qdl_init_dev(unsigned int segment, unsigned int bus, unsigned int device, unsigned int function)
{
	qdl_struct *dscr_data = NULL;
	qdl_dscr_t dscr = NULL;
	bool supported = false;
	int status = QDL_SUCCESS;

	dscr_data = malloc(sizeof(qdl_struct));
	if(dscr_data == NULL) {
		QDL_DEBUGLOG_FUNCTION_FAIL("malloc", 0);
		return NULL;
	}
	memset(dscr_data, 0, sizeof(qdl_struct));
	dscr = (qdl_dscr_t)dscr_data;

	/* Initialize descriptor with PCI location */
	dscr_data->pci.seg = segment;
	dscr_data->pci.bus = bus;
	dscr_data->pci.dev = device;
	dscr_data->pci.fun = function;

	/* Open socket for Devlink */
	status = _qdl_open_socket(dscr);
	if(status != QDL_SUCCESS) {
		QDL_DEBUGLOG_FUNCTION_FAIL("_qdl_open_socket", status);
		qdl_release_dev(dscr);
		return NULL;
	}

	/* Initialize QDL descriptor */
	status = _qdl_init_dscr(dscr);
	if(status != QDL_SUCCESS) {
		QDL_DEBUGLOG_FUNCTION_FAIL("_qdl_init_dscr", status);
		qdl_release_dev(dscr);
		return NULL;
	}

	/* Verify if device supports Devlink */
	status = _qdl_is_dev_supported(dscr, &supported);
	if(status != QDL_SUCCESS) {
		QDL_DEBUGLOG_FUNCTION_FAIL("_qdl_is_dev_supported", status);
		qdl_release_dev(dscr);
		return NULL;
	}
	if(supported == false) {
		qdl_release_dev(dscr);
		return NULL;
	}

	return dscr;
}

