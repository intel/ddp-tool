/*************************************************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2020 - 2021 Intel Corporation.
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

/* Snapshot ID defines */
#define QDL_FLASH_SNAPSHOT_ID                         9                    /* QDL snapshot ID value for flash */
#define QDL_CAPS_SNAPSHOT_ID                          19                   /* QDL snapshot ID value for caps */
#define QDL_INVALID_SNAPSHOT_ID                       0xFFFFFFFF           /* invalid snapshot ID value */

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

	QDL_DEBUGLOG_ENTERING;

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

	sprintf(buffer, "%04x:%02x:%02x.%1x", dscr_data->pci.seg, dscr_data->pci.bus, dscr_data->pci.dev,
		dscr_data->pci.fun);
	return buffer;
}

/**
 * _qdl_get_snapshot_id
 * @dscr: QDL descriptor
 * @region: region name
 *
 * Gets snapshot ID for provided region. In the case region is not supported QDL_INVALID_SNAPSHOT_ID is
 * returned.
 * Returns snapshot ID.
 */
uint32_t _qdl_get_snapshot_id(qdl_dscr_t dscr, char* region)
{
	qdl_struct *dscr_data = (qdl_struct*)dscr;

	if(strcmp(region, QDL_REGION_NAME_FLASH) == 0) {
		return dscr_data->flash_region.snapshot_id;
	} else if(strcmp(region, QDL_REGION_NAME_CAPS) == 0) {
		return dscr_data->caps_region.snapshot_id;
	} else {
		return QDL_INVALID_SNAPSHOT_ID;
	}
}

/**
 * _qdl_get_next_dev_msg
 * @dscr: QDL descriptor
 * @buff: buffer with messages
 * @buff_size: buffer size
 * @msg: pointer to current message
 *
 * Gets next message address for provided device 'dscr'.
 * Returns message address if success, otherwise error code.
 */
uint8_t* _qdl_get_next_dev_msg(qdl_dscr_t dscr, uint8_t *buff, uint32_t buff_size, uint8_t *msg)
{
	char location[QDL_DEV_LOCATION_SIZE];
	qdl_struct *dscr_data = (qdl_struct*)dscr;
	qdl_status_t status = QDL_INVALID_PARAMS;
	unsigned int msg_size = 0;
	unsigned int segment = 0;
	unsigned int bus = 0;
	unsigned int device = 0;
	unsigned int function = 0;

	/* Initialize */
	memset(location, '\0', QDL_DEV_LOCATION_SIZE);

	/* Find message for the device */
	msg = _qdl_get_next_msg(buff, buff_size, msg);
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
 * _qdl_init_region
 * @dscr: QDL descriptor
 * @region: Data defining region
 *
 * Initializes region resources.
 * Returns QDL_SUCCESS if function succeeds, otherwise error code.
 */
qdl_status_t _qdl_init_region(qdl_dscr_t dscr, qdl_region_t* region)
{
	uint8_t *rec_buff = NULL;
	uint8_t *msg = NULL;
	qdl_status_t status = QDL_SUCCESS;
	uint32_t snapshot_id = 0;
	unsigned int rec_buff_size = QDL_REC_BUFF_SIZE;
	unsigned int msg_size = 0;

	QDL_DEBUGLOG_ENTERING;

	/* Check if snapshot is created for region */
	rec_buff = malloc(rec_buff_size);
	if(rec_buff == NULL) {
		QDL_DEBUGLOG_FUNCTION_FAIL("malloc", 0);
		return QDL_MEMORY_ERROR;
	}
	memset(rec_buff, 0, rec_buff_size);
	status = qdl_receive_reply_msg(dscr, QDL_CMD_REGION_GET, region, rec_buff, &rec_buff_size);
	if(status != QDL_SUCCESS) {
		QDL_DEBUGLOG_FUNCTION_FAIL("qdl_receive_reply_msg", status);
		free(rec_buff);
		return status;
	}

	msg = _qdl_get_next_msg(rec_buff, rec_buff_size, NULL);
	msg_size = rec_buff + rec_buff_size - msg;
	status = _qdl_get_int_nattr_by_type(msg, msg_size, QDL_DEVLINK_ATTR_REGION_SNAPSHOT_ID, &snapshot_id);
	if(status == QDL_SUCCESS) {
		region->snapshot_id = snapshot_id;
	} else {
		/* No snapshot for region, create one */
		status = qdl_receive_reply_msg(dscr, QDL_CMD_REGION_NEW, region, NULL, NULL);
		if(status != QDL_SUCCESS) {
			QDL_DEBUGLOG_FUNCTION_FAIL("qdl_receive_reply_msg", status);
			region->snapshot_id = QDL_INVALID_SNAPSHOT_ID;
			free(rec_buff);
			return QDL_INIT_ERROR;
		}
		region->new_snapshot_id = true;
	}
	free(rec_buff);

	return QDL_SUCCESS;
}

/**
 * _qdl_init_dscr
 * @dscr: QDL descriptor
 * @flags: indicates which resources should be initialized
 *
 * Initializes QDL descriptor setting appropriate family ID.
 * Returns QDL_SUCCESS if function succeeds, otherwise error code.
 */
qdl_status_t _qdl_init_dscr(qdl_dscr_t dscr, uint32_t flags)
{
	qdl_struct *qdl_dscr = (qdl_struct*)dscr;
	qdl_status_t status = QDL_SUCCESS;
	unsigned int read_bytes = 0;

	QDL_DEBUGLOG_ENTERING;

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

	/* Initialize optional resources */
	if(flags & QDL_INIT_NVM) {
		qdl_dscr->flash_region.name = QDL_REGION_NAME_FLASH;
		qdl_dscr->flash_region.snapshot_id = QDL_FLASH_SNAPSHOT_ID;
		status = _qdl_init_region(dscr, &qdl_dscr->flash_region);
		if(status != QDL_SUCCESS) {
			QDL_DEBUGLOG_FUNCTION_FAIL("_qdl_init_region (flash)", status);
			return status;
		}
	}
	if(flags & QDL_INIT_CAPS) {
		qdl_dscr->caps_region.name = QDL_REGION_NAME_CAPS;
		qdl_dscr->caps_region.snapshot_id = QDL_CAPS_SNAPSHOT_ID;
		status = _qdl_init_region(dscr, &qdl_dscr->caps_region);
		if(status != QDL_SUCCESS) {
			QDL_DEBUGLOG_FUNCTION_FAIL("_qdl_init_region (caps)", status);
			return status;
		}
	}

	return QDL_SUCCESS;
}

/**
 * _qdl_read_msg_family_id
 * @dscr: QDL descriptor
 * @family_id: read family id
 *
 * Reads family ID for the device.
 * Returns QDL_SUCCESS if function succeeds, otherwise error code.
 */
qdl_status_t _qdl_read_msg_family_id(qdl_dscr_t dscr, uint32_t *family_id)
{
	uint8_t *send_buff = NULL;
	uint8_t *rec_buff = NULL;
	uint8_t *msg = NULL;
	qdl_status_t status = QDL_SUCCESS;
	unsigned int send_buff_size = 0;
	unsigned int rec_buff_size = 0;
	unsigned int msg_size = 0;

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
	status = _qdl_get_uint32_attr(msg, msg_size, CTRL_ATTR_FAMILY_ID, family_id);
	if(status != QDL_SUCCESS) {
		QDL_DEBUGLOG_FUNCTION_FAIL("_qdl_get_uint32_attr", status);
		free(rec_buff);
		return status;
	}
	free(rec_buff);

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
	msg = _qdl_get_next_dev_msg(dscr, rec_buff, rec_buff_size, NULL);
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

/**
 * qdl_receive_msg
 * @dscr: QDL descriptor
 * @rec_msg_buff: buffer for received message
 * @rec_msg_size: buffer size
 *
 * Receives message.
 * Returns QDL_SUCCESS if the function succeeds, otherwise an error code.
 */
qdl_status_t _qdl_receive_msg(qdl_dscr_t dscr, uint8_t *rec_msg_buff, unsigned int *rec_msg_size)
{
	struct msghdr rec_msg;
	struct iovec iov;
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

	return QDL_SUCCESS;
}

/*************************************************************************************************************
 * QDL API
 */

/**
 * qdl_get_string_by_type
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
qdl_status_t qdl_get_string_by_type(qdl_dscr_t dscr, uint8_t *buff, uint32_t buff_size, uint32_t type,
		char *string, unsigned int string_size)
{
	uint8_t *msg = NULL;
	qdl_status_t status = QDL_INVALID_PARAMS;
	unsigned int msg_size = 0;

	/* Validate input parameters */
	if(dscr == NULL || buff == NULL || buff_size == 0 || string == NULL || string_size == 0) {
		return status;
	}

	msg = _qdl_get_next_dev_msg(dscr, buff, buff_size, NULL);
	if(msg != NULL) {
		msg_size = buff + buff_size - msg;
		status = _qdl_get_string_attr(msg, msg_size, type, string, string_size);
	}

	return status;
}

/**
 * qdl_get_string_by_key
 * @dscr: QDL descriptor
 * @buff: buffer with messages
 * @buff_size: buffer size
 * key: key name for the searched value
 * @value: buffer for a value of the child nested attribute
 * @value_size: buffer size of the value
 *
 * Some nested attributes contains pair of child attributes organized as key and value. The function gets
 * value of child attribute based on provided key string.
 * Returns QDL_SUCCESS if success, otherwise error code.
 */
qdl_status_t qdl_get_string_by_key(qdl_dscr_t dscr, uint8_t *buff, uint32_t buff_size, char *key,
		char *value, int value_size)
{
	uint8_t *msg = NULL;
	qdl_status_t status = QDL_INVALID_PARAMS;
	unsigned int msg_size = 0;

	/* Validate input parameters */
	if(dscr == NULL || buff == NULL || buff_size == 0 || key == NULL || value == NULL
			|| value_size == 0) {
		return status;
	}

	msg = _qdl_get_next_dev_msg(dscr, buff, buff_size, NULL);
	if(msg != NULL) {
		msg_size = buff + buff_size - msg;
		status = _qdl_get_string_nattr_by_key(msg, msg_size, key, value, value_size);
	}

	return status;
}

/**
 * qdl_get_region_header_size
 * @data_size: region length which should be read
 *
 * Returns expected size of the headers for messages returning region content.
 */
uint32_t qdl_get_region_header_size(uint32_t data_size)
{
	return (data_size / (QDL_FLASH_CHUNK_MAX_SIZE * QDL_NUM_OF_MSG_FLASH_CHUNKS) + 1) * QDL_FLASH_CHUNK_HEADER_SIZE;
}

/**
 * qdl_read_region
 * @dscr: QDL descriptor
 * @msg_buff: buffer with messages
 * @msg_buff_size: buffer size
 * @offset: flash offset in bytes for NVM buffer to read
 * @bin_buff: returned binary buffer for attribute of type 'type'
 * @bin_size: buffer size for binary value and on exit size of the read binary data
 *
 * Gets binary buffer attribute based on argument 'offset' for specified device 'dscr' from several messages.
 * Returns QDL_SUCCESS if success, otherwise error code.
 */
qdl_status_t qdl_read_region(qdl_dscr_t dscr, uint8_t *msg_buff, uint32_t msg_buff_size, uint64_t offset,
			     uint8_t *bin_buff, unsigned int *bin_size)
{
	uint8_t *msg = NULL;
	uint64_t init_offset;
	qdl_status_t status = QDL_SUCCESS;
	unsigned int msg_size = 0;
	unsigned int buff_size = 0;
	unsigned int read_data = 0;
	bool first_msg = true;

	/* Validate input parameters */
	if(dscr == NULL || msg_buff == NULL || bin_buff == NULL || bin_size == NULL) {
		return QDL_INVALID_PARAMS;
	}

	msg = _qdl_get_next_dev_msg(dscr, msg_buff, msg_buff_size, NULL);
	while (msg != NULL) {
		/* Check if there is control message */
		if(_qdl_is_ctrl_msg((struct nlmsghdr*)msg)) {
			*bin_size = read_data;
			return _qdl_get_ctrl_msg_status((struct nlmsghdr*)msg);
		}

		/* Calculate address for message and buffer for data */
		msg_size = msg_buff + msg_buff_size - msg;
		buff_size = *bin_size - read_data;

		/* Get all flash data from message */
		status = _qdl_get_region(msg, msg_size, (bin_buff + read_data), &buff_size, &init_offset);
		if(status != QDL_SUCCESS) {
			return status;
		}

		/* Check if initial offset if correct */
		if(first_msg && init_offset != offset) {
			return QDL_CORRUPTED_MSG_ERROR;
		}
		first_msg = false;

		/* Check if all data was read */
		read_data += buff_size;
		if(*bin_size == read_data) {
			break;
		}

		msg = _qdl_get_next_msg(msg_buff, msg_buff_size, msg);
	}

	*bin_size = read_data;

	return QDL_SUCCESS;
}

/**
 * qdl_get_param_value
 * @dscr: QDL descriptor
 * @buff: buffer with messages
 * @buff_size: buffer size
 * @name: param name
 * @cmode: cmode for data value
 * @data: data buffer
 * @data_size; buffer size
 *
 * Gets data value for specified cmode type. The attribute structure for param command is described in
 * functionis _qdl_get_param_value().
 * Returns QDL_SUCCESS if success, otherwise error code.
 */
qdl_status_t qdl_get_param_value(qdl_dscr_t dscr, uint8_t *buff, uint32_t buff_size, char *name,
				 qdl_param_cmode_t cmode, uint8_t *data, unsigned int *data_size)
{
	char read_name[QDL_PARAM_NAME_SIZE];
	uint8_t *msg = NULL;
	qdl_status_t status = QDL_INVALID_PARAMS;
	uint32_t msg_size = 0;

	/* Initialize variables */
	memset(read_name, 0, sizeof(read_name));

	/* Validate input parameters */
	if(dscr == NULL || buff == NULL || buff_size == 0 || data == NULL || data_size == NULL) {
		return status;
	}

	msg = _qdl_get_next_dev_msg(dscr, buff, buff_size, NULL);
	while(msg != NULL) {
		/* Check if there is a control message */
		if(_qdl_is_ctrl_msg((struct nlmsghdr*)msg)) {
			*data_size = 0;
			return _qdl_get_ctrl_msg_status((struct nlmsghdr*)msg);
		}

		msg_size = buff + buff_size - msg;
		status = _qdl_get_string_attr(msg, msg_size, QDL_DEVLINK_ATTR_PARAM_NAME, read_name,
					      QDL_PARAM_NAME_SIZE);
		if(status != QDL_SUCCESS) {
			return QDL_PARSE_MSG_ERROR;
		}
		if(strcmp(name, read_name) == 0) {
			status = _qdl_get_param_value(msg, msg_size, cmode, data, data_size);
			break;
		}

		/* Get next message */
		msg = _qdl_get_next_dev_msg(dscr, buff, buff_size, msg);
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
 * qdl_receive_msgs
 * @dscr: QDL descriptor
 * @rec_buff: buffer for received messages
 * @rec_buff_size: buffer size
 *
 * Receives message.
 * Returns QDL_SUCCESS if the function succeeds, otherwise an error code.
 */
qdl_status_t qdl_receive_msg(qdl_dscr_t dscr, uint8_t *rec_buff, unsigned int *rec_buff_size)
{
	struct nlmsghdr *msg = NULL;
	uint8_t *msg_buff = NULL;
	qdl_status_t status = QDL_SUCCESS;
	unsigned int msgs_size = 0;
	unsigned int msg_size = 0;

	QDL_DEBUGLOG_ENTERING;

	/* Validate input parameters */
	if(dscr == NULL || rec_buff == NULL || rec_buff_size == NULL) {
		return QDL_INVALID_PARAMS;
	}

	msg_buff = rec_buff;
	msg_size = *rec_buff_size;
	do {
		status = _qdl_receive_msg(dscr, msg_buff, &msg_size);
		if(status != QDL_SUCCESS) {
			QDL_DEBUGLOG_FUNCTION_FAIL("_qdl_receive_msg", status);
			return status;
		}
		msgs_size += msg_size;

		/* Check if we got control message */
		msg = (struct nlmsghdr*)_qdl_get_next_msg(msg_buff, msg_size, NULL);
		while(msg != NULL) {
			if(_qdl_is_ctrl_msg(msg)) {
				*rec_buff_size = msgs_size;
				return _qdl_get_ctrl_msg_status(msg);
			}
			msg = (struct nlmsghdr*)_qdl_get_next_msg(msg_buff, msg_size, (uint8_t*)msg);
		}

		/* No control message. Receive next message. */
		msg_buff += msg_size;
		if(*rec_buff_size - msgs_size > 0) {
			msg_size = *rec_buff_size - msgs_size;
		} else {
			msg_size = 0;
		}
	} while(msg_size > 0);

	return QDL_BUFFER_TOO_SMALL_ERROR;
}

/**
 * qdl_receive_reply_msg
 * @dscr: QDL descriptor
 * @cmd_type: command type of message
 * @data: optional data for send message
 * @reply_buff: buffer for reply data
 * @rply_buff_size: reply buffer size
 *
 * Receives message for sent one. Function stories received data in reply_buff if provided. If reply buffer is
 * NULL function checks only if no error was returned for sent message. If so appropriate error is returned.
 * Returns QDL_SUCCESS if the function succeeds, otherwise an error code.
 */
qdl_status_t qdl_receive_reply_msg(qdl_dscr_t dscr, int cmd_type, void *data, uint8_t *reply_buff,
		unsigned int *reply_buff_size)
{
	uint8_t *send_buff = NULL;
	uint8_t *buff = NULL;
	uint8_t *rec_buff = NULL;
	qdl_status_t status = QDL_SUCCESS;
	unsigned int send_buff_size = 0;
	unsigned int rec_buff_size = 0;

	QDL_DEBUGLOG_ENTERING;

	/* Create message based on command type */
	send_buff = qdl_create_msg(dscr, cmd_type, &send_buff_size, data);
	if(send_buff == NULL) {
		QDL_DEBUGLOG_FUNCTION_FAIL("qdl_create_msg error", 0);
		return QDL_CREATE_MSG_ERROR;
	}

	/* Send message */
	status = qdl_send_msg(dscr, send_buff, send_buff_size);
	free(send_buff);
	if(status != QDL_SUCCESS) {
		QDL_DEBUGLOG_FUNCTION_FAIL("qdl_send_msg error", status);
		return status;
	}

	/* Check if reply buffer was provided */
	if(reply_buff != NULL && reply_buff_size != NULL) {
		rec_buff_size = *reply_buff_size;
		rec_buff = reply_buff;
	} else if(reply_buff != NULL || reply_buff_size != NULL) {
		QDL_DEBUGLOG_ERROR_MSG("Inconsistent input parameters.");
		return QDL_INVALID_PARAMS;
	} else {
		rec_buff_size = QDL_REC_BUFF_SIZE;
		buff = malloc(rec_buff_size);
		if(buff == NULL) {
			QDL_DEBUGLOG_FUNCTION_FAIL("malloc", 0);
			return QDL_MEMORY_ERROR;
		}
		memset(buff, 0, rec_buff_size);
		rec_buff = buff;
	}

	/* Receive reply for the sent message */
	status = qdl_receive_msg(dscr, rec_buff, &rec_buff_size);
	if(status != QDL_SUCCESS) {
		QDL_DEBUGLOG_FUNCTION_FAIL("qdl_receive_msg", status);
		rec_buff_size = 0;
	}
	if(reply_buff_size != NULL) {
		*reply_buff_size = rec_buff_size;
	}

	/* Release resources */
	if(buff != NULL) {
		free(buff);
	}

	return status;
}

/**
 * qdl_create_msg
 * @dscr: QDL descriptor
 * @cmd_type: command type of message
 * @msg_size: size of the created message
 * @data: optional message data dependent on cmd_type
 *
 * Allocates buffer for new message of type 'cmd_type' and fills with appropriate content. Notice that user
 * should free the allocated buffer.
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
	case QDL_CMD_INFO_GET:
		_qdl_put_msg_header(msg, dscr_data->id, NLM_F_REQUEST | NLM_F_ACK);
		_qdl_put_msg_extra_header(msg, cmd_type, 1);
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_BUS_NAME, "pci");
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_LOCATION, _qdl_get_bus_name(dscr, bus_name_buff));
		break;
	case QDL_CMD_PORT_GET:
		_qdl_put_msg_header(msg, dscr_data->id, NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP);
		_qdl_put_msg_extra_header(msg, cmd_type, 1);
		break;
	case QDL_CMD_PARAM_GET:
		if(data == NULL) {
			free(msg);
			return NULL;
		}
		_qdl_put_msg_header(msg, dscr_data->id, NLM_F_REQUEST | NLM_F_ACK);
		_qdl_put_msg_extra_header(msg, cmd_type, 1);
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_BUS_NAME, "pci");
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_LOCATION, _qdl_get_bus_name(dscr, bus_name_buff));
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_PARAM_NAME, (char*)data);
		break;
	case QDL_CMD_PARAM_SET:
		if(data == NULL) {
			free(msg);
			return NULL;
		}
		_qdl_put_msg_header(msg, dscr_data->id, NLM_F_REQUEST | NLM_F_ACK);
		_qdl_put_msg_extra_header(msg, cmd_type, 1);
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_BUS_NAME, "pci");
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_LOCATION, _qdl_get_bus_name(dscr, bus_name_buff));
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_PARAM_NAME,
				      ((qdl_msg_param_set_t*)data)->minsrev_name);
		_qdl_put_msg_uint8_attr(msg, QDL_DEVLINK_ATTR_PARAM_VALUE_CMODE, QDL_PARAM_CMODE_PERMANENT);
		_qdl_put_msg_uint8_attr(msg, QDL_DEVLINK_ATTR_PARAM_TYPE, QDL_ATTR_TYPE_UINT32);
		_qdl_put_msg_dynamic_attr(msg, QDL_DEVLINK_ATTR_PARAM_VALUE_DATA,
					  &((qdl_msg_param_set_t*)data)->minsrev_value,
					  sizeof(uint32_t));
		break;
	case QDL_CMD_REGION_GET:
		if(data == NULL || _qdl_validate_region_name(((qdl_region_t*)data)->name) != QDL_SUCCESS) {
			free(msg);
			return NULL;
		}
		_qdl_put_msg_header(msg, dscr_data->id, NLM_F_REQUEST | NLM_F_ACK);
		_qdl_put_msg_extra_header(msg, cmd_type, 1);
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_BUS_NAME, "pci");
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_LOCATION, _qdl_get_bus_name(dscr, bus_name_buff));
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_REGION_NAME, ((qdl_region_t*)data)->name);
		break;
	case QDL_CMD_REGION_NEW:
	case QDL_CMD_REGION_DEL:
		if(data == NULL || _qdl_validate_region_name(((qdl_region_t*)data)->name) != QDL_SUCCESS) {
			free(msg);
			return NULL;
		}
		_qdl_put_msg_header(msg, dscr_data->id, NLM_F_REQUEST | NLM_F_ACK);
		_qdl_put_msg_extra_header(msg, cmd_type, 1);
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_BUS_NAME, "pci");
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_LOCATION, _qdl_get_bus_name(dscr, bus_name_buff));
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_REGION_NAME, ((qdl_region_t*)data)->name);
		_qdl_put_msg_uint32_attr(msg, QDL_DEVLINK_ATTR_REGION_SNAPSHOT_ID,
				         ((qdl_region_t*)data)->snapshot_id);
		break;
	case QDL_CMD_REGION_READ:
		if(data == NULL ||
		   _qdl_validate_region_name(((qdl_msg_region_read_t*)data)->region) != QDL_SUCCESS) {
			free(msg);
			return NULL;
		}
		_qdl_put_msg_header(msg, dscr_data->id, NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP);
		_qdl_put_msg_extra_header(msg, cmd_type, 1);
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_BUS_NAME, "pci");
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_LOCATION, _qdl_get_bus_name(dscr, bus_name_buff));
		_qdl_put_msg_str_attr(msg, QDL_DEVLINK_ATTR_REGION_NAME,
				      ((qdl_msg_region_read_t*)data)->region);
		_qdl_put_msg_uint32_attr(msg, QDL_DEVLINK_ATTR_REGION_SNAPSHOT_ID,
					 _qdl_get_snapshot_id(dscr, ((qdl_msg_region_read_t*)data)->region));
		_qdl_put_msg_uint64_attr(msg, QDL_DEVLINK_ATTR_REGION_CHUNK_ADDR,
					 ((qdl_msg_region_read_t*)data)->address);
		_qdl_put_msg_uint64_attr(msg, QDL_DEVLINK_ATTR_REGION_CHUNK_LEN,
					 ((qdl_msg_region_read_t*)data)->length);
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
	qdl_struct *dscr_data = (qdl_struct*)dscr;
	qdl_status_t status = QDL_SUCCESS;

	QDL_DEBUGLOG_ENTERING;

	if(dscr_data != NULL) {
		/* Delete region snapshot ID created by QDL */
		if(dscr_data->flash_region.new_snapshot_id == true) {
			status = qdl_receive_reply_msg(dscr, QDL_CMD_REGION_DEL, &dscr_data->flash_region,
						       NULL, NULL);
			if(status != QDL_SUCCESS) {
				QDL_DEBUGLOG_FUNCTION_FAIL("qdl_receive_reply_msg", status);
			}
			dscr_data->flash_region.snapshot_id = QDL_INVALID_SNAPSHOT_ID;
			dscr_data->flash_region.new_snapshot_id = false;
		}
		if(dscr_data->caps_region.new_snapshot_id == true) {
			status = qdl_receive_reply_msg(dscr, QDL_CMD_REGION_DEL, &dscr_data->caps_region,
						       NULL, /* buffer for reply data */
						       NULL  /* reply buffer size */);
			if(status != QDL_SUCCESS) {
				QDL_DEBUGLOG_FUNCTION_FAIL("qdl_receive_reply_msg", status);
			}
			dscr_data->caps_region.snapshot_id = QDL_INVALID_SNAPSHOT_ID;
			dscr_data->caps_region.new_snapshot_id = false;
		}

		/* Release socket */
		dscr_data->socket = QDL_INVALID_SOCKET;
		qdl_socket_count--;
		if(qdl_socket_count == 0) {
			close(qdl_socket);
			qdl_socket = QDL_SOCKET_ERROR;
		}
		free(dscr_data);
	}
}

/**
 * qdl_init_dev
 * @segment: PCIe segment of the device to initialize
 * @bus: PCIe bus of the device to initialize
 * @device: PCIe device of the device to initialize
 * @function: PCIe function of the device to initialize
 * @flags: indicates which resources should be initialized
 *
 * Initializes Devlink resources and opens socket to communicate with base driver.
 * Returns address to the QDL descriptor if the function succeeds, otherwise an error code.
 */
qdl_dscr_t qdl_init_dev(unsigned int segment, unsigned int bus, unsigned int device, unsigned int function,
			unsigned int flags)
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

	/* Initialize others descriptor fields */
	dscr_data->flash_region.snapshot_id = QDL_INVALID_SNAPSHOT_ID;
	dscr_data->flash_region.new_snapshot_id = false;
	dscr_data->caps_region.snapshot_id = QDL_INVALID_SNAPSHOT_ID;
	dscr_data->caps_region.new_snapshot_id = false;

	/* Open socket for Devlink */
	status = _qdl_open_socket(dscr);
	if(status != QDL_SUCCESS) {
		QDL_DEBUGLOG_FUNCTION_FAIL("_qdl_open_socket", status);
		qdl_release_dev(dscr);
		return NULL;
	}

	/* Read message type */
	status = _qdl_read_msg_family_id(dscr, &dscr_data->id);
	if(status != QDL_SUCCESS) {
		QDL_DEBUGLOG_FUNCTION_FAIL("_qdl_read_msg_family_id", status);
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

	/* Initialize QDL descriptor */
	status = _qdl_init_dscr(dscr, flags);
	if(status != QDL_SUCCESS) {
		QDL_DEBUGLOG_FUNCTION_FAIL("_qdl_init_dscr", status);
		qdl_release_dev(dscr);
		return NULL;
	}

	return dscr;
}

