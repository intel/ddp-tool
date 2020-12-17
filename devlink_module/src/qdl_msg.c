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

#include "qdl_msg.h"
#include "qdl_debug.h"
#include "qdl_codes.h"
#include "qdl_t.h"
#include <linux/genetlink.h>
#include <linux/netlink.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define QDL_DEVLINK_NAME  "devlink"

/*
 * Devlink message
 *
 * 0x0            0x15             0x31
 * +------------ header ------------+
 * |           nlmsg_len            |
 * +--------------------------------+
 * |  nlmsg_type   |  nlmsg_flags   |
 * +--------------------------------+
 * |            nlmsg_seq           |
 * +--------------------------------+
 * |            nlmsg_pid           |
 * +--------------------------------+
 * +--------- extra header ---------+
 * |  cmd  |  ver  |    reserved    |
 * +--------------------------------+
 * +--------- attributes -----------+
 * |   nla_len     |   nla_type     |
 * +--------------------------------+
 * |  attribute payload ............|
 * +--------------------------------+
 * | ...............................|
 * +--------------------------------+
 */

typedef struct {
	uint32_t id;
	qdl_attr_type_t type;
} qdl_attr_field_t;

static qdl_attr_field_t qdl_ctrl_attr_table[] = {
	{CTRL_ATTR_FAMILY_ID,                       QDL_ATTR_TYPE_UINT16},
	{CTRL_ATTR_FAMILY_NAME,                     QDL_ATTR_TYPE_STRING},
	{CTRL_ATTR_VERSION,                         QDL_ATTR_TYPE_UINT32},
	{CTRL_ATTR_HDRSIZE,                         QDL_ATTR_TYPE_UINT32},
	{CTRL_ATTR_MAXATTR,                         QDL_ATTR_TYPE_UINT32},
	{CTRL_ATTR_OPS,                             QDL_ATTR_TYPE_NESTED},
	{CTRL_ATTR_MCAST_GROUPS,                    QDL_ATTR_TYPE_MCAST_GROUP},
	{__CTRL_ATTR_MAX,                           QDL_ATTR_TYPE_INVALID}
};

static qdl_attr_field_t qdl_devlink_attr_table[] = {
	{QDL_DEVLINK_ATTR_BUS_NAME,                 QDL_ATTR_TYPE_STRING},
	{QDL_DEVLINK_ATTR_LOCATION,                 QDL_ATTR_TYPE_STRING},
	{QDL_DEVLINK_ATTR_PORT_INDEX,               QDL_ATTR_TYPE_UINT32},
	{QDL_DEVLINK_ATTR_PORT_TYPE,                QDL_ATTR_TYPE_UINT16},
	{QDL_DEVLINK_ATTR_DESIRED_TYPE,             QDL_ATTR_TYPE_UINT16},
	{QDL_DEVLINK_ATTR_NETDEV_IF_INDEX,          QDL_ATTR_TYPE_UINT32},
	{QDL_DEVLINK_ATTR_NETDEV_NAME,              QDL_ATTR_TYPE_STRING},
	{QDL_DEVLINK_ATTR_PORT_FLAVOUR,             QDL_ATTR_TYPE_UINT16},
	{QDL_DEVLINK_ATTR_PORT_NUMBER,              QDL_ATTR_TYPE_UINT32},
	{QDL_DEVLINK_ATTR_REGION_NAME,              QDL_ATTR_TYPE_STRING},
	{QDL_DEVLINK_ATTR_INFO_DRIVER_NAME,         QDL_ATTR_TYPE_STRING},
	{QDL_DEVLINK_ATTR_INFO_SERIAL_NUMBER,       QDL_ATTR_TYPE_STRING},
	{QDL_DEVLINK_ATTR_INFO_VERSION_FIXED,       QDL_ATTR_TYPE_NESTED},
	{QDL_DEVLINK_ATTR_INFO_VERSION_RUNNING,     QDL_ATTR_TYPE_NESTED},
	{QDL_DEVLINK_ATTR_INFO_VERSION_STORED,      QDL_ATTR_TYPE_NESTED},
	{QDL_DEVLINK_ATTR_INFO_VERSION_NAME,        QDL_ATTR_TYPE_STRING},
	{QDL_DEVLINK_ATTR_INFO_VERSION_VALUE,       QDL_ATTR_TYPE_STRING},
	{QDL_DEVLINK_ATTR_FLASH_UPDATE_FILE_NAME,   QDL_ATTR_TYPE_STRING},
	{QDL_DEVLINK_ATTR_ID_END,                   QDL_ATTR_TYPE_INVALID}
};

/**
 * _qdl_get_msg_size
 * @cmd_type: command type for the message
 *
 * Returns message size.
 */
int _qdl_get_msg_size(int cmd_type)
{
	int size = 0;
	int header_size = sizeof(struct nlmsghdr);
	int extra_header_size = sizeof(struct genlmsghdr);
	int str_attr_header_size = NLA_HDRLEN;

	switch(cmd_type) {
	case QDL_CMD_GET:
		size = header_size + extra_header_size + 2 * str_attr_header_size + QDL_BUS_NAME_LENGTH + \
		       QDL_PCI_LOCATION_NAME_LENGTH;
		break;
	case QDL_CMD_PORT_GET:
	case QDL_CMD_INFO_GET:
		size = header_size + extra_header_size;
		break;
	case QDL_CMD_FLASH_UPDATE:
		size = header_size + extra_header_size + 3 * str_attr_header_size + QDL_BUS_NAME_LENGTH + \
		       QDL_PCI_LOCATION_NAME_LENGTH + QDL_FILE_NAME_MAX_LENGTH;
		break;
	case CTRL_CMD_GETFAMILY:
		size = header_size + extra_header_size + str_attr_header_size + strlen(QDL_DEVLINK_NAME) + 1;
		break;
	default:
		size = 0;
	}

	return size;
}

/**
 * _qdl_get_next_msg
 * @buff: buffer containing Devlink messages.
 * @buff_size: buffer size
 * @msg: pointer to current message
 *
 * Returns address for the next message in the buffer buff.
 */
uint8_t* _qdl_get_next_msg(uint8_t *buff, unsigned int buff_size, uint8_t *msg)
{
	struct nlmsghdr *header = NULL;
	uint8_t *next_msg = NULL;

	QDL_DEBUGLOG_ENTERING;

	if(msg == NULL) {
		return buff;
	}

	header = (struct nlmsghdr*)msg;
	next_msg = msg + header->nlmsg_len;
	if(next_msg >= buff + buff_size) {
		next_msg = NULL;
	}

	return next_msg;
}

/**
 * _qdl_get_msg_tail
 * @msg: message buffer
 *
 * Returns next byte after message tail.
 */
uint8_t* _qdl_get_msg_tail(uint8_t *msg)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;

	return (header == NULL) ? NULL : (uint8_t*)(msg + header->nlmsg_len);
}

/**
 * _qdl_is_ctrl_msg
 * @msg: message
 *
 * Returns true if provided message is control one, otherwise false.
 */
bool _qdl_is_ctrl_msg(struct nlmsghdr *msg)
{
	bool is_ctrl = false;

	switch(msg->nlmsg_type) {
	case NLMSG_NOOP:
	case NLMSG_ERROR:
	case NLMSG_DONE:
	case NLMSG_OVERRUN:
		is_ctrl = true;
		break;
	default:
		is_ctrl = false;
	}

	return is_ctrl;
}

/**
 * _qdl_get_msg_data_addr
 * @msg: message buffer
 *
 * Returns address of message payload.
 */
uint8_t* _qdl_get_msg_data_addr(uint8_t *msg)
{
	return msg + NLMSG_HDRLEN;
}

/**
 * _qdl_get_extra_header_addr
 * @msg: message buffer
 *
 * Returns address for the extra header in the message if present, otherwise NULL.
 */
uint8_t* _qdl_get_extra_header_addr(uint8_t *msg)
{
	if(msg == NULL) {
		return NULL;
	}

	if(_qdl_is_ctrl_msg((struct nlmsghdr*)msg) == false) {
		return _qdl_get_msg_data_addr(msg);
	}

	return NULL;
}

/**
 * _qdl_get_ctr_attr_type
 * @attr: attribute
 * @msg_type: message type
 *
 * Returns type of the provided attribute for control messages.
 */
qdl_attr_type_t _qdl_get_ctr_attr_type(struct nlattr *attr)
{
	int i = 0;

	for(i = 0; qdl_ctrl_attr_table[i].id < __CTRL_ATTR_MAX; i++) {
		if(attr->nla_type == qdl_ctrl_attr_table[i].id) {
			return qdl_ctrl_attr_table[i].type;
		}
	}

	return QDL_ATTR_TYPE_INVALID;
}

/**
 * _qdl_get_devlink_attr_type
 * @attr: attribute
 * @msg_type: message type
 *
 * Returns type of the provided attribute for devlink messages.
 */
qdl_attr_type_t _qdl_get_devlink_attr_type(struct nlattr *attr)
{
	int i = 0;

	for(i = 0; qdl_devlink_attr_table[i].id < QDL_DEVLINK_ATTR_ID_END; i++) {
		if(attr->nla_type == qdl_devlink_attr_table[i].id) {
			return qdl_devlink_attr_table[i].type;
		}
	}

	return QDL_ATTR_TYPE_INVALID;
}

/**
 * _qdl_get_attr_type
 * @attr: attribute
 * @msg_type: message type
 *
 * Returns type of the provided attribute for provided message type.
 */
qdl_attr_type_t _qdl_get_attr_type(struct nlattr *attr, uint16_t msg_type)
{
	if(msg_type == GENL_ID_CTRL) {
		return _qdl_get_ctr_attr_type(attr);
	} else if (msg_type > GENL_ID_CTRL){
		return _qdl_get_devlink_attr_type(attr);
	} else {
		return QDL_ATTR_TYPE_INVALID;
	}
}

/**
 * _qdl_is_uint16_attr
 * @attr: attribute which needs to be checked
 * @msg_type: message type
 *
 * Returns TRUE if provided attribute is of type uint16 for provided message type, otherwise FALSE.
 */
bool _qdl_is_uint16_attr(struct nlattr *attr, uint16_t msg_type)
{
	return (_qdl_get_attr_type(attr, msg_type) == QDL_ATTR_TYPE_UINT16);
}

/**
 * _qdl_is_uint32_attr
 * @attr: attribute which needs to be checked
 * @msg_type: message type
 *
 * Returns TRUE if provided attribute is of type uint32 for provided message type, otherwise FALSE.
 */
bool _qdl_is_uint32_attr(struct nlattr *attr, uint16_t msg_type)
{
	return (_qdl_get_attr_type(attr, msg_type) == QDL_ATTR_TYPE_UINT32);
}

/**
 * _qdl_is_string_attr
 * @attr: attribute which needs to be checked
 * @msg_type: message type
 *
 * Returns TRUE if provided attribute is of type string for provided message type, otherwise FALSE.
 */
bool _qdl_is_string_attr(struct nlattr *attr, uint16_t msg_type)
{
	return (_qdl_get_attr_type(attr, msg_type) == QDL_ATTR_TYPE_STRING);
}

/**
 * _qdl_is_nested_attr
 * @attr: attribute which needs to be checked
 * @msg_type: message type
 *
 * Returns TRUE if provided attribute is nested for provided message type, otherwise FALSE.
 */
bool _qdl_is_nested_attr(struct nlattr *attr, uint16_t msg_type)
{
	return (_qdl_get_attr_type(attr, msg_type) == QDL_ATTR_TYPE_NESTED);
}

/**
 * _qdl_get_first_attr_addr
 * @msg: message buffer
 * @msg_size: message size
 *
 * Returns address for the first attribute in the message if present, otherwise NULL.
 */
uint8_t* _qdl_get_first_attr_addr(uint8_t *msg, uint32_t msg_size)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;
	struct genlmsghdr *extra_header = (struct genlmsghdr*)_qdl_get_extra_header_addr(msg);
	uint8_t *attr_addr = NULL;

	/* Validate input parameters */
	if(header == NULL) {
		return NULL;
	}
	if(msg_size < header->nlmsg_len) {
		return NULL;
	}

	/* Control messages do not contain attributes */
	if(_qdl_is_ctrl_msg(header)) {
		return NULL;
	}

	/* Calculate attribute address */
	if(extra_header == NULL && header->nlmsg_len > NLMSG_HDRLEN) {
		attr_addr = _qdl_get_msg_data_addr(msg);
	} else if(extra_header != NULL && header->nlmsg_len > NLMSG_HDRLEN + GENL_HDRLEN) {
		attr_addr = (uint8_t*)extra_header + GENL_HDRLEN;
	}

	return attr_addr;
}

/**
 * _qdl_get_next_attr_addr
 * @msg: message buffer
 * @msg_size: message size
 * @attr_addr: address of the last valid attribute
 *
 * Returns address for the next attribute in the message if present, otherwise NULL.
 */
uint8_t* _qdl_get_next_attr_addr(uint8_t *msg, uint32_t msg_size, uint8_t *attr_addr)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;
	struct nlattr *attr = (struct nlattr*)attr_addr;
	uint8_t *next_attr_addr = NULL;

	/* Validate input parameters */
	if(header == NULL || attr == NULL) {
		return NULL;
	}
	if(msg_size < header->nlmsg_len) {
		return NULL;
	}
	if(attr->nla_len < NLA_HDRLEN) {
		return NULL;
	}

	/* Calculate next_attr_addr */
	if(msg + header->nlmsg_len > attr_addr + attr->nla_len) {
		next_attr_addr = attr_addr + NLA_ALIGN(attr->nla_len);
	}

	return next_attr_addr;
}

/**
 * _qdl_get_next_nested_attr_addr
 * @attr_addr: address of the parent attribute containing nested attributes
 * @nested_attr_addr: address of the current nested attribute or NULL if it is first function call
 *
 * Returns address for the next nested attribute in the message if present, otherwise NULL.
 */
uint8_t* _qdl_get_next_nested_attr_addr(uint8_t *attr_addr, uint8_t *nested_attr_addr)
{
	struct nlattr *attr = (struct nlattr*)attr_addr;
	struct nlattr *nested_attr = (struct nlattr*)nested_attr_addr;
	uint8_t *next_nested_attr_addr = NULL;

	/* Validate input parameters */
	if(attr == NULL) {
		return NULL;
	}

	/* Calculate next_attr_addr */
	if(nested_attr == NULL) {
		next_nested_attr_addr = (attr->nla_len > NLA_HDRLEN) ? attr_addr + NLA_HDRLEN : NULL;
	} else {
		if(nested_attr->nla_len <= NLA_HDRLEN) {
			return NULL;
		}
		next_nested_attr_addr = nested_attr_addr + NLA_ALIGN(nested_attr->nla_len);
		if(next_nested_attr_addr >= attr_addr + attr->nla_len) {
			return NULL;
		}
	}

	return next_nested_attr_addr;
}

/**
 * _qdl_get_attr_data
 * @msg: message buffer
 * @msg_size: buffer size
 * @attr_addr: address of the last valid attribute
 *
 * Returns address for the attribute data if present, otherwise NULL.
 */
uint8_t* _qdl_get_attr_data_addr(uint8_t *msg, uint32_t msg_size, uint8_t *attr_addr)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;
	struct nlattr *attr = (struct nlattr*)attr_addr;
	uint8_t *data_addr = NULL;

	/* Validate input parameters */
	if(header == NULL || attr == NULL) {
		return NULL;
	}
	if(attr->nla_len < NLA_HDRLEN || msg_size < header->nlmsg_len) {
		return NULL;
	}

	/* Calculate data address */
	if(msg + header->nlmsg_len >= attr_addr + attr->nla_len) {
		data_addr = attr_addr + sizeof(*attr);
	}

	return data_addr;
}

/**
 * _qdl_get_attr_addr
 * @msg: message buffer
 * @msg_size: message size
 * @type: address of the last valid attribute
 *
 * Returns address for the next attribute in the message if present, otherwise NULL.
 */
uint8_t* _qdl_get_attr_addr(uint8_t *msg, uint32_t msg_size, uint32_t type)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;
	struct nlattr *attr = NULL;
	struct nlattr *nested_attr = NULL;

	attr = (struct nlattr*)_qdl_get_first_attr_addr(msg, msg_size);
	while(attr != NULL) {
		if(attr->nla_type == type) {
			return (uint8_t*)attr;
		}

		/* Check if this is a nested attribute */
		if(_qdl_is_nested_attr(attr, header->nlmsg_type)) {
			nested_attr = (struct nlattr*)_qdl_get_next_nested_attr_addr((uint8_t*)attr, NULL);
			while(nested_attr != NULL) {
				if(nested_attr->nla_type == type) {
					return (uint8_t*)nested_attr;
				}

				/* Get next nested attribute */
				nested_attr = (struct nlattr*)_qdl_get_next_nested_attr_addr(
						(uint8_t*)attr, (uint8_t*)nested_attr);
			}
		}

		/* Get next attribute */
		attr = (struct nlattr*)_qdl_get_next_attr_addr(msg, msg_size, (uint8_t*)attr);
	}

	return NULL;
}

/**
 * _qdl_get_nested_attr_value_addr
 * @msg: message buffer
 * @msg_size: message size
 * @name: nested attribute name
 *
 * Returns address for the nested attribute value based on nested attribute name if it is present,
 * otherwise NULL.
 */
uint8_t* _qdl_get_nested_attr_value_addr(uint8_t *msg, uint32_t msg_size, char *name)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;
	struct nlattr *attr = NULL;
	struct nlattr *nested_attr = NULL;
	uint8_t *data = NULL;

	attr = (struct nlattr*)_qdl_get_first_attr_addr(msg, msg_size);
	while(attr != NULL) {
		if(_qdl_is_nested_attr(attr, header->nlmsg_type)) {
			nested_attr = (struct nlattr*)_qdl_get_next_nested_attr_addr((uint8_t*)attr, NULL);
			data = _qdl_get_attr_data_addr(msg, msg_size, (uint8_t*)nested_attr);
			if(data != NULL && strcmp(name, (char*)data) == 0) {
				return _qdl_get_next_nested_attr_addr((uint8_t*)attr, (uint8_t*)nested_attr);
			}
		}
		attr = (struct nlattr*)_qdl_get_next_attr_addr(msg, msg_size, (uint8_t*)attr);
	}

	return NULL;
}

/**
 * _qdl_get_uint32_attr
 * @msg: message buffer
 * @msg_size: buffer size
 * @type: attribute type
 * @value: uint32 value for attribute of type 'type'
 *
 * Gets attribute data as uint32 value based on argument 'type'.
 * Returns QDL_SUCCESS if success, otherwise error code.
 */
qdl_status_t _qdl_get_uint32_attr(uint8_t *msg, uint32_t msg_size, uint32_t type, uint32_t *value)
{
	uint8_t *attr_addr = NULL;
	uint8_t *data = NULL;

	attr_addr = _qdl_get_attr_addr(msg, msg_size, type);
	data = _qdl_get_attr_data_addr(msg, msg_size, attr_addr);
	if(data != NULL) {
		*value = (uint32_t)*data;
		return QDL_SUCCESS;
	}

	return QDL_PARSE_MSG_ERROR;
}

/**
 * _qdl_get_string_attr
 * @msg: message buffer
 * @msg_size: buffer size
 * @type: attribute type
 * @string: returned string for attribute of type 'type'
 * @string_size; buffer size for string
 *
 * Gets attribute data as a string value based on argument 'type'.
 * Returns QDL_SUCCESS if success, otherwise error code.
 */
qdl_status_t _qdl_get_string_attr(uint8_t *msg, uint32_t msg_size, uint32_t type, char *string,
		int string_size)
{
	uint8_t *attr_addr = NULL;
	uint8_t *data = NULL;

	attr_addr = _qdl_get_attr_addr(msg, msg_size, type);
	data = _qdl_get_attr_data_addr(msg, msg_size, attr_addr);
	if(data != NULL) {
		strncpy(string, (char*)data, string_size);
		return QDL_SUCCESS;
	}

	return QDL_PARSE_MSG_ERROR;
}

/**
 * _qdl_get_string_nested_attr
 * @msg: message buffer
 * @msg_size: buffer size
 * @name: name of the nested attribute
 * @value: buffer for a value of the nested attribute
 * @value_size: buffer size of the value
 *
 * Gets attribute's value based on attribute's name for nested attribute.
 * Returns QDL_SUCCESS if success, otherwise error code.
 */
qdl_status_t _qdl_get_string_nested_attr(uint8_t *msg, uint32_t msg_size, char *name, char *value,
		int value_size)
{
	uint8_t *attr_addr = NULL;
	uint8_t *data = NULL;

	attr_addr = _qdl_get_nested_attr_value_addr(msg, msg_size, name);
	data = _qdl_get_attr_data_addr(msg, msg_size, attr_addr);
	if(data != NULL) {
		strncpy(value, (char*)data, value_size);
		return QDL_SUCCESS;
	}

	return QDL_PARSE_MSG_ERROR;
}

/**
 * _qdl_put_msg_str_attr
 * @msg: message buffer
 * @type: attribute type
 * @string: string to add
 *
 * Fills message buffer with attribute of type string.
 * Returns pointer to the message buffer if success, otherwise NULL.
 */
uint8_t* _qdl_put_msg_str_attr(uint8_t *msg, uint16_t type, char *string)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;
	struct nlattr *attr = (struct nlattr*)_qdl_get_msg_tail(msg);
	uint8_t *data = (uint8_t*)attr + NLA_HDRLEN;
	int length = NLA_ALIGN(strlen(string) + 1) + NLA_HDRLEN;

	/* Validate parameters */
	if(attr == NULL || header == NULL) {
		return NULL;
	}

	attr->nla_len = length;
	attr->nla_type = type;
	memcpy(data, string, strlen(string) + 1);
	header->nlmsg_len += length;

	return msg;
}

/**
 * _qdl_put_msg_extra_header
 * @msg: message buffer
 * @cmd: command field
 * @version: version field
 *
 * Fills message buffer with extra header content. It can be called only once per message.
 * Returns pointer to the message buffer if success, otherwise error.
 */
uint8_t* _qdl_put_msg_extra_header(uint8_t *msg, uint8_t cmd, uint8_t version)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;
	struct genlmsghdr *extra_header = (struct genlmsghdr*)_qdl_get_msg_tail(msg);

	/* Validate parameters */
	if(header == NULL || extra_header == NULL) {
		return NULL;
	}

	/* Fill extra header */
	extra_header->cmd = cmd;
	extra_header->version = version;
	header->nlmsg_len += NLA_ALIGN(sizeof(*extra_header));

	return msg;
}

/**
 * _qdl_put_msg_header
 * @msg: message buffer
 * @type: message type
 * @flags: message flags
 *
 * Fills message buffer with header content.
 * Returns pointer to the message buffer if success, otherwise error.
 */
uint8_t* _qdl_put_msg_header(uint8_t *msg, uint16_t type, uint16_t flags)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;

	/* Validate parameters */
	if(header == NULL) {
		return NULL;
	}

	/* Fill header */
	header->nlmsg_len = NLA_ALIGN(sizeof(*header));
	header->nlmsg_type = type;
	header->nlmsg_flags = flags;
	header->nlmsg_seq = time(NULL);
	header->nlmsg_pid = 0;

	return msg;
}

/**
 * _qdl_create_generic_msg
 * @id: message ID
 * @cmd_type: command type of message
 * @msg_size: size of the created message
 *
 * Allocates buffer for message of the type 'cmd_type' and fills with appropriate content.
 * Returns address to the created message if success, otherwise NULL.
 */
uint8_t* _qdl_create_generic_msg(int id, int cmd_type, unsigned int *msg_size)
{
	uint8_t *msg = NULL;

	QDL_DEBUGLOG_ENTERING;

	*msg_size = _qdl_get_msg_size(cmd_type);
	if(*msg_size == 0) {
		QDL_DEBUGLOG_FUNCTION_FAIL("qdl_get_msg_size", (int)*msg_size);
		return NULL;
	}
	msg = malloc(*msg_size);
	if(msg == NULL) {
		QDL_DEBUGLOG_FUNCTION_FAIL("malloc", 0);
		return NULL;
	}
	memset(msg, 0, *msg_size);
	switch(cmd_type) {
	case CTRL_CMD_GETFAMILY:
		_qdl_put_msg_header(msg, id, NLM_F_REQUEST | NLM_F_ACK);
		_qdl_put_msg_extra_header(msg, cmd_type, 1);
		_qdl_put_msg_str_attr(msg, CTRL_ATTR_FAMILY_NAME, QDL_DEVLINK_NAME);
		break;
	default:
		free(msg);
		msg = NULL;
		*msg_size = 0;
	}

	return msg;
}

/**
 * _qdl_print_header
 * @fp: file pointer
 * @header_buff: message buffer
 *
 * Prints formatted message header.
 */
void _qdl_print_header(FILE *fp, uint8_t *msg)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;

	fprintf(fp, "-------- header ---------\n");
	fprintf(fp, "length:  0x%08X (%d)\n", header->nlmsg_len, header->nlmsg_len);
	fprintf(fp, "type:    0x%04X\n", header->nlmsg_type);
	fprintf(fp, "flags:   0x%04X\n", header->nlmsg_flags);
	fprintf(fp, "seq:     0x%08X\n", header->nlmsg_seq);
	fprintf(fp, "pid:     0x%08X\n", header->nlmsg_pid);
}

/**
 * _qdl_print_ctrl
 * @fp: file pointer
 * @msg: message buffer
 *
 * Prints formatted payload for control messages.
 */
void _qdl_print_ctrl_data(FILE *fp, uint8_t *msg)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;
	struct nlmsgerr *err_data = NULL;

	switch(header->nlmsg_type) {
	case NLMSG_ERROR:
		err_data = (struct nlmsgerr*)_qdl_get_msg_data_addr(msg);
		fprintf(fp, "-------- error ----------\n");
		fprintf(fp, "error:   %d\n", err_data->error);
		_qdl_print_msg(fp, (uint8_t*)(&err_data->msg), err_data->msg.nlmsg_len);
		break;
	default:
		fprintf(fp, "Unknown control message\n");
	}
}

/**
 * _qdl_print_attr
 * @fp: file pointer
 * @title: title string for attribute
 * @msg: message buffer
 * @msg_size: buffer size
 * @attr: attribute to print
 *
 * Prints formatted attribute.
 */
void _qdl_print_attr(FILE *fp, char *title, uint8_t *msg, uint32_t msg_size, struct nlattr *attr)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;
	struct nlattr *nested_attr = NULL;
	uint8_t *data = NULL;
	unsigned int i = 0;

	data = _qdl_get_attr_data_addr(msg, msg_size, (uint8_t*)attr);
	fprintf(fp, "-------- %11s ----\n", title);
	fprintf(fp, "len:     0x%04X (%d)\n", attr->nla_len, attr->nla_len);
	fprintf(fp, "type:    0x%04X\n", attr->nla_type);
	fprintf(fp, "value:   ");
	if(data != NULL) {
		if(_qdl_is_uint16_attr(attr, header->nlmsg_type)) {
			fprintf(fp, "%02X (%d)\n", (uint16_t)(*data), (uint16_t)(*data));
		} else if(_qdl_is_uint32_attr(attr, header->nlmsg_type)) {
			fprintf(fp, "%04X (%d)\n", (uint32_t)(*data), (uint32_t)(*data));
		} else if(_qdl_is_string_attr(attr, header->nlmsg_type)) {
			fprintf(fp, "'%s'\n", (char*)data);
		} else if(_qdl_is_nested_attr(attr, header->nlmsg_type)) {
			fprintf(fp, "<nested>\n");
			nested_attr = (struct nlattr*)_qdl_get_next_nested_attr_addr((uint8_t*)attr, NULL);
			while(nested_attr != NULL) {
				_qdl_print_attr(fp, "nested attr", msg, msg_size, nested_attr);
				nested_attr = (struct nlattr*)_qdl_get_next_nested_attr_addr(
						(uint8_t*)attr, (uint8_t*)nested_attr);
			}
		} else {
			for(i = 0; i < attr->nla_len - 4; i++) {
				fprintf(fp, "%02X ", data[i]);
			}
			fprintf(fp, "(unknown attr type)\n");
		}
	} else {
		fprintf(fp, "is NULL\n");
	}
}

/**
 * qdl_print_msg
 * @fp: file pointer
 * @msg: buffer for message to be created
 * @msg_size: message buffer size
 *
 * Prints formatted message.
 */
void _qdl_print_msg(FILE *fp, uint8_t *buff, unsigned int buff_size)
{
	struct nlmsghdr *header = NULL;
	struct genlmsghdr *extra_header = NULL;
	struct nlattr *attr = NULL;
	uint8_t *msg = buff;
	uint32_t msg_size = 0;

	QDL_DEBUGLOG_ENTERING;

	/* Validate input parameters */
	if(fp == NULL || buff == NULL) {
		return;
	}

	/* Print message content */
	while(msg != NULL) {
		/* Initialize */
		header = (struct nlmsghdr*)msg;
		extra_header = (struct genlmsghdr*)_qdl_get_extra_header_addr(msg);
		msg_size = header->nlmsg_len;

		/* Validate */
		if(msg_size > (buff + buff_size - msg)) {
			printf("Corrupted message\n");
			break;
		}
		if(msg_size < NLMSG_HDRLEN) {
			break;
		}

		/* Print header */
		_qdl_print_header(fp, msg);

		/* Print control message content */
		if(_qdl_is_ctrl_msg(header)) {
			_qdl_print_ctrl_data(fp, msg);
		}

		/* Print extended header if applicable */
		if(extra_header != NULL) {
			fprintf(fp, "-------- extra header ---\n");
			fprintf(fp, "cmd:     0x%02X\n", extra_header->cmd);
			fprintf(fp, "version: 0x%02X\n", extra_header->version);
		}

		/* Print attributes */
		attr = (struct nlattr*)_qdl_get_first_attr_addr(msg, msg_size);
		while(attr != NULL) {
			_qdl_print_attr(fp, "attr", msg, msg_size, attr);
			attr = (struct nlattr*)_qdl_get_next_attr_addr(msg, msg_size, (uint8_t*)attr);
		}
		msg = _qdl_get_next_msg(buff, buff_size, msg);
	}
}
