/*************************************************************************************************************
* Copyright (C) 2020 Intel Corporation                                                                       *
*                                                                                                            *
* Redistribution and use in source and binary forms, with or without modification, are permitted provided    *
* that the following conditions are met:                                                                     *
*                                                                                                            *
* 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the  *
*    following disclaimer.                                                                                   *
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and   *
*      the following disclaimer in the documentation and/or other materials provided with the distribution.  *
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or    *
*    promote products derived from this software without specific prior written permission.                  *
*                                                                                                            *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED     *
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A     *
* PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR   *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED *
* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)  *
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING   *
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE        *
* POSSIBILITY OF SUCH DAMAGE.                                                                                *
*                                                                                                            *
* SPDX-License-Identifier: BSD-3-Clause                                                                      *
*************************************************************************************************************/

#include "qdl_msg.h"
#include "qdl_debug.h"
#include "qdl_codes.h"
#include "qdl_t.h"
#include <linux/genetlink.h>
#include <linux/netlink.h>
#include <inttypes.h>
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
	{QDL_DEVLINK_ATTR_PARAM,                    QDL_ATTR_TYPE_NESTED},
	{QDL_DEVLINK_ATTR_PARAM_NAME,               QDL_ATTR_TYPE_STRING},
	{QDL_DEVLINK_ATTR_PARAM_TYPE,               QDL_ATTR_TYPE_UINT8},
	{QDL_DEVLINK_ATTR_PARAM_VALUES_LIST,        QDL_ATTR_TYPE_NESTED},
	{QDL_DEVLINK_ATTR_PARAM_VALUE,              QDL_ATTR_TYPE_NESTED},
	{QDL_DEVLINK_ATTR_PARAM_VALUE_DATA,         QDL_ATTR_TYPE_DYNAMIC},
	{QDL_DEVLINK_ATTR_PARAM_VALUE_CMODE,        QDL_ATTR_TYPE_UINT8},
	{QDL_DEVLINK_ATTR_REGION_NAME,              QDL_ATTR_TYPE_STRING},
	{QDL_DEVLINK_ATTR_REGION_SIZE,              QDL_ATTR_TYPE_UINT64},
	{QDL_DEVLINK_ATTR_REGION_SNAPSHOTS,         QDL_ATTR_TYPE_NESTED},
	{QDL_DEVLINK_ATTR_REGION_SNAPSHOT,          QDL_ATTR_TYPE_NESTED},
	{QDL_DEVLINK_ATTR_REGION_SNAPSHOT_ID,       QDL_ATTR_TYPE_UINT32},
	{QDL_DEVLINK_ATTR_REGION_CHUNKS,            QDL_ATTR_TYPE_NESTED},
	{QDL_DEVLINK_ATTR_REGION_CHUNK,             QDL_ATTR_TYPE_NESTED},
	{QDL_DEVLINK_ATTR_REGION_CHUNK_DATA,        QDL_ATTR_TYPE_BINARY},
	{QDL_DEVLINK_ATTR_REGION_CHUNK_ADDR,        QDL_ATTR_TYPE_UINT64},
	{QDL_DEVLINK_ATTR_REGION_CHUNK_LEN,         QDL_ATTR_TYPE_UINT64},
	{QDL_DEVLINK_ATTR_INFO_DRIVER_NAME,         QDL_ATTR_TYPE_STRING},
	{QDL_DEVLINK_ATTR_INFO_SERIAL_NUMBER,       QDL_ATTR_TYPE_STRING},
	{QDL_DEVLINK_ATTR_INFO_VERSION_FIXED,       QDL_ATTR_TYPE_NESTED},
	{QDL_DEVLINK_ATTR_INFO_VERSION_RUNNING,     QDL_ATTR_TYPE_NESTED},
	{QDL_DEVLINK_ATTR_INFO_VERSION_STORED,      QDL_ATTR_TYPE_NESTED},
	{QDL_DEVLINK_ATTR_INFO_VERSION_NAME,        QDL_ATTR_TYPE_STRING},
	{QDL_DEVLINK_ATTR_INFO_VERSION_VALUE,       QDL_ATTR_TYPE_STRING},
	{QDL_DEVLINK_ATTR_FLASH_UPDATE_FILE_NAME,   QDL_ATTR_TYPE_STRING},
	{QDL_DEVLINK_ATTR_RELOAD_ACTION,            QDL_ATTR_TYPE_UINT8},
	{QDL_DEVLINK_ATTR_ID_END,                   QDL_ATTR_TYPE_INVALID}
};

static char* qdl_region_name[] = {
	QDL_REGION_NAME_FLASH,
	QDL_REGION_NAME_CAPS,
	NULL
};

/**
 * _qdl_validate_region_name
 * @name: region name
 *
 * Checks if QDL supports provided region type.
 * Returns QDL_SUCCESS if region supported, otherwise QDL_INVALID_PARAMS.
 */
qdl_status_t _qdl_validate_region_name(char* name)
{
	int i = 0;

	/* Validate 'name' variable */
	if(name == NULL) {
		return QDL_INVALID_PARAMS;
	}

	/* Check if region is supported */
	for(i = 0; qdl_region_name[i] != NULL; i++) {
		if(strcmp(name, qdl_region_name[i]) == 0) {
			return QDL_SUCCESS;
		}
	}

	return QDL_INVALID_PARAMS;
}

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
	case QDL_CMD_INFO_GET:
		size = header_size + extra_header_size + 2 * str_attr_header_size + QDL_BUS_NAME_LENGTH + \
		       QDL_PCI_LOCATION_NAME_LENGTH;
		break;
	case QDL_CMD_PORT_GET:
		size = header_size + extra_header_size;
		break;
	case QDL_CMD_RELOAD:
		size = header_size + extra_header_size + 3 * str_attr_header_size + QDL_BUS_NAME_LENGTH + \
			   QDL_PCI_LOCATION_NAME_LENGTH + NLA_ALIGN(sizeof(uint8_t));
		break;
	case QDL_CMD_PARAM_GET:
		size = header_size + extra_header_size + 3 * str_attr_header_size + QDL_BUS_NAME_LENGTH + \
		       QDL_PCI_LOCATION_NAME_LENGTH + NLA_ALIGN(strlen(QDL_FW_SREV_NAME) + 1);
		break;
	case QDL_CMD_PARAM_SET:
		size = header_size + extra_header_size + 6 * str_attr_header_size + QDL_BUS_NAME_LENGTH + \
		       QDL_PCI_LOCATION_NAME_LENGTH + NLA_ALIGN(strlen(QDL_FW_SREV_NAME) + 1) + \
		       2 * NLA_ALIGN(sizeof(uint8_t)) + NLA_ALIGN(sizeof(uint32_t));
		break;
	case QDL_CMD_REGION_GET:
		size = header_size + extra_header_size + 3 * str_attr_header_size + QDL_BUS_NAME_LENGTH + \
		       QDL_PCI_LOCATION_NAME_LENGTH + NLA_ALIGN(QDL_REGION_NAME_SIZE);
		break;
	case QDL_CMD_REGION_NEW:
	case QDL_CMD_REGION_DEL:
		size = header_size + extra_header_size + 4 * str_attr_header_size + QDL_BUS_NAME_LENGTH + \
		       QDL_PCI_LOCATION_NAME_LENGTH + NLA_ALIGN(QDL_REGION_NAME_SIZE) + sizeof(uint32_t);
		break;
	case QDL_CMD_REGION_READ:
		size = header_size + extra_header_size + 6 * str_attr_header_size + QDL_BUS_NAME_LENGTH + \
		       QDL_PCI_LOCATION_NAME_LENGTH + NLA_ALIGN(QDL_REGION_NAME_SIZE) + sizeof(uint32_t) + \
		       2 * sizeof(uint64_t);
		break;
	case QDL_CMD_FLASH_UPDATE:
		size = header_size + extra_header_size + 3 * str_attr_header_size + QDL_BUS_NAME_LENGTH + \
		       QDL_PCI_LOCATION_NAME_LENGTH + QDL_FILE_NAME_MAX_LENGTH;
		break;
	case CTRL_CMD_GETFAMILY:
		size = header_size + extra_header_size + str_attr_header_size + \
		       NLA_ALIGN(strlen(QDL_DEVLINK_NAME) + 1);
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
 * _qdl_is_uint8_attr
 * @attr: attribute which needs to be checked
 * @msg_type: message type
 *
 * Returns TRUE if provided attribute is of type uint8 for provided message type, otherwise FALSE.
 */
bool _qdl_is_uint8_attr(struct nlattr *attr, uint16_t msg_type)
{
	return (_qdl_get_attr_type(attr, msg_type) == QDL_ATTR_TYPE_UINT8);
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
 * _qdl_is_uint64_attr
 * @attr: attribute which needs to be checked
 * @msg_type: message type
 *
 * Returns TRUE if provided attribute is of type uint64 for provided message type, otherwise FALSE.
 */
bool _qdl_is_uint64_attr(struct nlattr *attr, uint16_t msg_type)
{
	return (_qdl_get_attr_type(attr, msg_type) == QDL_ATTR_TYPE_UINT64);
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
 * _qdl_is_binary_attr
 * @attr: attribute which needs to be checked
 * @msg_type: message type
 *
 * Returns TRUE if provided attribute is of type binary for provided message type, otherwise FALSE.
 */
bool _qdl_is_binary_attr(struct nlattr *attr, uint16_t msg_type)
{
	return (_qdl_get_attr_type(attr, msg_type) == QDL_ATTR_TYPE_BINARY);
}

/**
 * _qdl_is_dynamic_attr
 * @attr: attribute which needs to be checked
 * @msg_type: message type
 *
 * Returns TRUE if provided attribute is of type dynamic for provided message type, otherwise FALSE.
 */
bool _qdl_is_dynamic_attr(struct nlattr *attr, uint16_t msg_type)
{
	return (_qdl_get_attr_type(attr, msg_type) == QDL_ATTR_TYPE_DYNAMIC);
}

/**
 * _qdl_is_nattr
 * @attr: attribute which needs to be checked
 * @msg_type: message type
 *
 * Returns TRUE if provided attribute is nested for provided message type, otherwise FALSE.
 */
bool _qdl_is_nattr(struct nlattr *attr, uint16_t msg_type)
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
 * _qdl_get_next_nattr_addr
 * @attr_addr: address of the parent attribute containing nested attributes
 * @nattr_addr: address of the current nested attribute or NULL if it is first function call
 *
 * Returns address for the next nested attribute in the message if present, otherwise NULL.
 */
uint8_t* _qdl_get_next_nattr_addr(uint8_t *attr_addr, uint8_t *nattr_addr)
{
	struct nlattr *attr = (struct nlattr*)attr_addr;
	struct nlattr *nattr = (struct nlattr*)nattr_addr;
	uint8_t *next_nattr_addr = NULL;

	/* Validate input parameters */
	if(attr == NULL) {
		return NULL;
	}

	/* Calculate next_attr_addr */
	if(nattr == NULL) {
		next_nattr_addr = (attr->nla_len > NLA_HDRLEN) ? attr_addr + NLA_HDRLEN : NULL;
	} else {
		if(nattr->nla_len <= NLA_HDRLEN) {
			return NULL;
		}
		next_nattr_addr = nattr_addr + NLA_ALIGN(nattr->nla_len);
		if(next_nattr_addr >= attr_addr + attr->nla_len) {
			return NULL;
		}
	}

	return next_nattr_addr;
}

/**
 * _qdl_get_next_nattr_addr_by_type
 * @attr_addr: address of the parent attribute containing nested attributes
 * @type: attribute type to look for
 * @nattr_addr: address of the current nested attribute or NULL if it is first function call
 *
 * Returns address for the next attribute in the nested one based on type. If no attribute found return NULL.
 */
uint8_t* _qdl_get_next_nattr_addr_by_type(uint8_t *attr_addr, uint32_t type, uint8_t *nattr_addr)
{
	struct nlattr *attr = (struct nlattr*)attr_addr;
	uint8_t *next_nattr_addr = NULL;

	/* Validate input parameters */
	if(attr == NULL) {
		return NULL;
	}

	next_nattr_addr = _qdl_get_next_nattr_addr(attr_addr, nattr_addr);
	while(next_nattr_addr != NULL) {
		if(((struct nlattr*)next_nattr_addr)->nla_type == type) {
			return next_nattr_addr;
		}
		next_nattr_addr = _qdl_get_next_nattr_addr(attr_addr, next_nattr_addr);
	}

	return NULL;
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
	struct nlattr *nattr = NULL;

	attr = (struct nlattr*)_qdl_get_first_attr_addr(msg, msg_size);
	while(attr != NULL) {
		if(attr->nla_type == type) {
			return (uint8_t*)attr;
		}

		/* Check if this is a nested attribute */
		if(_qdl_is_nattr(attr, header->nlmsg_type)) {
			nattr = (struct nlattr*)_qdl_get_next_nattr_addr((uint8_t*)attr, NULL);
			while(nattr != NULL) {
				if(nattr->nla_type == type) {
					return (uint8_t*)nattr;
				}

				/* Get next nested attribute */
				nattr = (struct nlattr*)_qdl_get_next_nattr_addr(
						(uint8_t*)attr, (uint8_t*)nattr);
			}
		}

		/* Get next attribute */
		attr = (struct nlattr*)_qdl_get_next_attr_addr(msg, msg_size, (uint8_t*)attr);
	}

	return NULL;
}

/**
 * _qdl_get_nattr_value_addr_by_key
 * @msg: message buffer
 * @msg_size: message size
 * key: key string for nested attribute
 *
 * Returns address for the nested attribute based on nested attribute name if it is present,
 * otherwise NULL.
 */
uint8_t* _qdl_get_nattr_value_addr_by_key(uint8_t *msg, uint32_t msg_size, char *key)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;
	struct nlattr *attr = NULL;
	struct nlattr *nattr = NULL;
	uint8_t *data = NULL;

	attr = (struct nlattr*)_qdl_get_first_attr_addr(msg, msg_size);
	while(attr != NULL) {
		if(_qdl_is_nattr(attr, header->nlmsg_type)) {
			nattr = (struct nlattr*)_qdl_get_next_nattr_addr((uint8_t*)attr, NULL);
			data = _qdl_get_attr_data_addr(msg, msg_size, (uint8_t*)nattr);
			if(data != NULL && strcmp(key, (char*)data) == 0) {
				return _qdl_get_next_nattr_addr((uint8_t*)attr, (uint8_t*)nattr);
			}
		}
		attr = (struct nlattr*)_qdl_get_next_attr_addr(msg, msg_size, (uint8_t*)attr);
	}

	return NULL;
}

/**
 * _qdl_get_nattr_value_addr_by_type
 * @msg: message buffer
 * @msg_size: message size
 * @type: nested attribute type
 *
 * Returns address for the nested attribute based on nested attribute type if it is present,
 * otherwise NULL.
 */
uint8_t* _qdl_get_nattr_value_addr_by_type(uint8_t *msg, uint32_t msg_size, uint32_t type)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;
	struct nlattr *attr = NULL;
	struct nlattr *next_attr = NULL;

	attr = (struct nlattr*)_qdl_get_first_attr_addr(msg, msg_size);
	while(attr != NULL) {
		if(_qdl_is_nattr(attr, header->nlmsg_type)) {
			/* Check if attribute value is a next nested attribute */
			next_attr = (struct nlattr*)_qdl_get_attr_data_addr(msg, msg_size, (uint8_t*)attr);
			if(next_attr != NULL && _qdl_is_nattr(next_attr, header->nlmsg_type)) {
				next_attr = (struct nlattr*)_qdl_get_attr_data_addr(msg, msg_size, (uint8_t*)next_attr);
				if(next_attr != NULL && ((struct nlattr*)next_attr)->nla_type == type) {
					return (uint8_t*)next_attr;
				}
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
		unsigned int string_size)
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
 * _qdl_get_binary_attr
 * @msg: message buffer
 * @msg_size: buffer size
 * @type: attribute type
 * @bin_buff: returned binary buffer for attribute of type 'type'
 * @bin_buff_size: buffer size for binary value
 *
 * Gets attribute data as a string value based on argument 'type'.
 * Returns QDL_SUCCESS if success, otherwise error code.
 */
qdl_status_t _qdl_get_binary_attr(uint8_t *msg, uint32_t msg_size, uint32_t type, char *bin_buff,
		unsigned int bin_buff_size)
{
	uint8_t *attr_addr = NULL;
	uint8_t *data = NULL;

	attr_addr = _qdl_get_attr_addr(msg, msg_size, type);
	data = _qdl_get_attr_data_addr(msg, msg_size, attr_addr);
	if(data != NULL) {
		memcpy(bin_buff, (char*)data, bin_buff_size);
		return QDL_SUCCESS;
	}

	return QDL_PARSE_MSG_ERROR;
}

/**
 * _qdl_get_string_nattr_by_key
 * @msg: message buffer
 * @msg_size: buffer size
 * key: name of the child key attribute
 * @value: buffer for a value of the nested attribute
 * @value_size: buffer size of the value
 *
 * Some nested attributes contain pair of child attributes organized as key and value. The function gets value
 * of child attribute based on provided key string. The structure of the nested attribute:
 * - [attribute] <nested>
 *   -> [attribute] <string> # key
 *   -> [attribute] <string> # value
 * Returns QDL_SUCCESS if success, otherwise error code.
 */
qdl_status_t _qdl_get_string_nattr_by_key(uint8_t *msg, uint32_t msg_size, char *key, char *value,
		unsigned int value_size)
{
	uint8_t *attr_addr = NULL;
	uint8_t *data = NULL;

	attr_addr = _qdl_get_nattr_value_addr_by_key(msg, msg_size, key);
	data = _qdl_get_attr_data_addr(msg, msg_size, attr_addr);
	if(data != NULL) {
		strncpy(value, (char*)data, value_size);
		return QDL_SUCCESS;
	}

	return QDL_PARSE_MSG_ERROR;
}

/**
 * _qdl_get_int_nattr_by_type
 * @msg: message buffer
 * @msg_size: buffer size
 * @type: type of the nested attribute
 * @value: buffer for a value of the nested attribute
 * @value_size: buffer size of the value
 *
 * Gets attribute's integer value based on attribute's type for nested attribute.
 * Returns QDL_SUCCESS if success, otherwise error code.
 */
qdl_status_t _qdl_get_int_nattr_by_type(uint8_t *msg, uint32_t msg_size, uint32_t type, uint32_t *value)
{
	uint8_t *attr_addr = NULL;
	uint8_t *data = NULL;

	attr_addr = _qdl_get_nattr_value_addr_by_type(msg, msg_size, type);
	data = _qdl_get_attr_data_addr(msg, msg_size, attr_addr);
	if(data != NULL) {
		*value = *((uint32_t*)data);
		return QDL_SUCCESS;
	}

	return QDL_PARSE_MSG_ERROR;
}

/**
 * _qdl_get_region
 * @msg: buffer with messages
 * @msg_size: buffer size
 * @bin_buff: buffer for read NVM
 * @bin_size: buffer size and on exit size of the read binary data
 * @init_offset: initial offset of read flash
 *
 * Collects all binary buffer attributes from message and copies them to bin_buff parameter. Function checks
 * if collected data is continuous. If not QDL_CORRUPTED_MSG_ERROR error is returned. The attribute structure
 * for NVM buffer is as follows:
 * - QDL_DEVLINK_ATTR_REGION_CHUNKS <nested>
 *   -> QDL_DEVLINK_ATTR_REGION_CHUNK <nested>
 *      -> QDL_DEVLINK_ATTR_REGION_CHUNK_DATA <binary> # 1st data chunk of the max size 0x100 bytes
 *      -> QDL_DEVLINK_ATTR_REGION_CHUNK_ADDR <uint64> # starting address for the 1st chunk
 *      -> QDL_DEVLINK_ATTR_REGION_CHUNK_DATA <binary> # 2nd data chunk of the max size 0x100 bytes
 *      -> QDL_DEVLINK_ATTR_REGION_CHUNK_ADDR <uint64> # starting address for the 2nd chunk
 *      ...
 *      -> QDL_DEVLINK_ATTR_REGION_CHUNK_DATA <binary> # last data chunk of the remaining size
 *      -> QDL_DEVLINK_ATTR_REGION_CHUNK_ADDR <uint64> # starting address for the last chunk
 * Returns QDL_SUCCESS if success, otherwise error code.
 */
qdl_status_t _qdl_get_region(uint8_t *msg, uint32_t msg_size, uint8_t *bin_buff, unsigned int *bin_size,
			     uint64_t *init_offset)
{
	uint8_t *chunks_addr = NULL;
	uint8_t *chunk_addr = NULL;
	uint8_t *offset_addr = NULL;
	uint8_t *data_addr = NULL;
	uint8_t *data = NULL;
	uint64_t *read_offset = NULL;
	qdl_status_t status = QDL_SUCCESS;
	unsigned int chunk_size = 0;
	unsigned int read_data = 0;
	bool first_chunk = true;

	/* Find nested attribute QDL_DEVLINK_ATTR_REGION_CHUNKS */
	chunks_addr = _qdl_get_attr_addr(msg, msg_size, QDL_DEVLINK_ATTR_REGION_CHUNKS);
	if(chunks_addr == NULL) {
		return QDL_PARSE_MSG_ERROR;
	}

	/* Find QDL_DEVLINK_ATTR_REGION_CHUNK attribute with appropriate offset */
	chunk_addr = _qdl_get_next_nattr_addr_by_type(chunks_addr, QDL_DEVLINK_ATTR_REGION_CHUNK, NULL);
	while(chunk_addr != NULL) {
		offset_addr = _qdl_get_next_nattr_addr_by_type(chunk_addr, QDL_DEVLINK_ATTR_REGION_CHUNK_ADDR,
							       NULL);
		if(offset_addr == NULL) {
			return QDL_PARSE_MSG_ERROR;
		}

		/* Compare chunk data offset with the one requested */
		read_offset = (uint64_t*)_qdl_get_attr_data_addr(msg, msg_size, offset_addr);
		if(read_offset == NULL) {
			return QDL_PARSE_MSG_ERROR;
		}
		if(first_chunk) {
			*init_offset = *read_offset;
			first_chunk = false;
		}
		if(*read_offset == *init_offset + read_data) {
			/* Find attribute QDL_DEVLINK_ATTR_REGION_CHUNK_DATA */
			data_addr = _qdl_get_next_nattr_addr_by_type(chunk_addr,
								     QDL_DEVLINK_ATTR_REGION_CHUNK_DATA,
								     NULL);
			if(data_addr == NULL) {
				return QDL_PARSE_MSG_ERROR;
			}

			/* Copy binary data if buffer size is correct */
			chunk_size = ((struct nlattr*)data_addr)->nla_len - NLA_HDRLEN;
			if(chunk_size > *bin_size - read_data) {
				chunk_size = *bin_size - read_data;
				status = QDL_BUFFER_TOO_SMALL_ERROR;
			}
			data = (uint8_t*)_qdl_get_attr_data_addr(msg, msg_size, data_addr);
			if(data == NULL) {
				return QDL_PARSE_MSG_ERROR;
			}
			memcpy((bin_buff + read_data), data, chunk_size);
			read_data += chunk_size;

			/* Check if any error occurred */
			if(status != QDL_SUCCESS) {
				*bin_size = read_data;
				return status;
			}
		} else {
			return QDL_CORRUPTED_MSG_ERROR;
		}

		chunk_addr = _qdl_get_next_nattr_addr_by_type(chunks_addr, QDL_DEVLINK_ATTR_REGION_CHUNK,
							      chunk_addr);
	}

	*bin_size = read_data;

	return QDL_SUCCESS;
}

/**
 * _qdl_get_param_value
 * @msg: message buffer
 * @msg_size: buffer size
 * @cmode: cmode for data value
 * @data: data buffer
 * @data_size; buffer size
 *
 * Gets data value for specified cmode type. The attribute structure for param command is as follows:
 * - QDL_DEVLINK_ATTR_PARAM_VALUES_LIST <nested>
 *   -> QDL_DEVLINK_ATTR_PARAM_VALUE <nested>
 *      -> QDL_DEVLINK_ATTR_PARAM_VALUE_CMODE
 *      -> QDL_DEVLINK_ATTR_PARAM_VALUE_DATA
 *   -> QDL_DEVLINK_ATTR_PARAM_VALUE <nested>
 *      -> QDL_DEVLINK_ATTR_PARAM_VALUE_CMODE
 *      -> QDL_DEVLINK_ATTR_PARAM_VALUE_DATA
 *   ...
 *
 * Returns QDL_SUCCESS if success, otherwise error code.
 */
qdl_status_t _qdl_get_param_value(uint8_t *msg, uint32_t msg_size, uint8_t cmode, uint8_t *data,
				  unsigned int *data_size)
{
	uint8_t *list_addr = NULL;
	uint8_t *value_addr = NULL;
	uint8_t *cmode_addr = NULL;
	uint8_t *data_addr = NULL;
	uint8_t *data_buff_addr = NULL;
	uint8_t *read_cmode = NULL;
	qdl_status_t status = QDL_SUCCESS;
	unsigned int read_data_size = 0;

	/* Find nested attribute QDL_DEVLINK_ATTR_PARAM_VALUES_LIST */
	list_addr = _qdl_get_attr_addr(msg, msg_size, QDL_DEVLINK_ATTR_PARAM_VALUES_LIST);
	if(list_addr == NULL) {
		return QDL_PARSE_MSG_ERROR;
	}

	/* Find QDL_DEVLINK_ATTR_PARAM_VALUE attribute with appropriate cmode value */
	value_addr = _qdl_get_next_nattr_addr_by_type(list_addr, QDL_DEVLINK_ATTR_PARAM_VALUE, NULL);
	while(value_addr != NULL) {
		cmode_addr = _qdl_get_next_nattr_addr_by_type(value_addr, QDL_DEVLINK_ATTR_PARAM_VALUE_CMODE,
							      NULL);
		if(cmode_addr == NULL) {
			return QDL_PARSE_MSG_ERROR;
		}

		read_cmode = _qdl_get_attr_data_addr(msg, msg_size, cmode_addr);
		if(read_cmode != NULL && *read_cmode == cmode) {
			/* Find attribute QDL_DEVLINK_ATTR_PARAM_VALUE_DATA */
			data_addr = _qdl_get_next_nattr_addr_by_type(value_addr,
								     QDL_DEVLINK_ATTR_PARAM_VALUE_DATA,
								     NULL);
			if(data_addr == NULL) {
				return QDL_PARSE_MSG_ERROR;
			}

			/* Copy binary data if buffer size is correct */
			read_data_size = ((struct nlattr*)data_addr)->nla_len - NLA_HDRLEN;
			if(read_data_size > *data_size) {
				read_data_size = *data_size;
				status = QDL_BUFFER_TOO_SMALL_ERROR;
			}
			data_buff_addr = _qdl_get_attr_data_addr(msg, msg_size, data_addr);
			if(data_buff_addr == NULL) {
				return QDL_PARSE_MSG_ERROR;
			}
			memcpy(data, data_buff_addr, read_data_size);

			*data_size = read_data_size;
			return status;
		}

		value_addr = _qdl_get_next_nattr_addr_by_type(list_addr, QDL_DEVLINK_ATTR_PARAM_VALUE,
							      value_addr);
	}

	return status;
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

	/* Fill attribute buffer */
	attr->nla_len = length;
	attr->nla_type = type;
	memcpy(data, string, strlen(string) + 1);
	header->nlmsg_len += length;

	return msg;
}

/**
 * _qdl_put_msg_uint8_attr
 * @msg: message buffer
 * @type: attribute type
 * @value: numeric value which needs to be added
 *
 * Fills message buffer with attribute of type unit8.
 * Returns pointer to the message buffer if success, otherwise NULL.
 */
uint8_t* _qdl_put_msg_uint8_attr(uint8_t *msg, uint16_t type, uint8_t value)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;
	struct nlattr *attr = (struct nlattr*)_qdl_get_msg_tail(msg);
	uint8_t *data = (uint8_t*)attr + NLA_HDRLEN;
	int length = sizeof(value) + NLA_HDRLEN;

	/* Validate parameters */
	if(attr == NULL || header == NULL) {
		return NULL;
	}

	/* Fill attribute buffer */
	attr->nla_len = length;
	attr->nla_type = type;
	memcpy(data, &value, sizeof(value));
	header->nlmsg_len += NLA_ALIGN(length);

	return msg;
}

/**
 * _qdl_put_msg_uint32_attr
 * @msg: message buffer
 * @type: attribute type
 * @value: numeric value which needs to be added
 *
 * Fills message buffer with attribute of type unit32.
 * Returns pointer to the message buffer if success, otherwise NULL.
 */
uint8_t* _qdl_put_msg_uint32_attr(uint8_t *msg, uint16_t type, uint32_t value)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;
	struct nlattr *attr = (struct nlattr*)_qdl_get_msg_tail(msg);
	uint8_t *data = (uint8_t*)attr + NLA_HDRLEN;
	int length = NLA_ALIGN(sizeof(value)) + NLA_HDRLEN;

	/* Validate parameters */
	if(attr == NULL || header == NULL) {
		return NULL;
	}

	/* Fill attribute buffer */
	attr->nla_len = length;
	attr->nla_type = type;
	memcpy(data, &value, sizeof(value));
	header->nlmsg_len += length;

	return msg;
}

/**
 * _qdl_put_msg_uint64_attr
 * @msg: message buffer
 * @type: attribute type
 * @value: numeric value to add
 *
 * Fills message buffer with attribute of type unit64.
 * Returns pointer to the message buffer if success, otherwise NULL.
 */
uint8_t* _qdl_put_msg_uint64_attr(uint8_t *msg, uint16_t type, uint64_t value)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;
	struct nlattr *attr = (struct nlattr*)_qdl_get_msg_tail(msg);
	uint8_t *data = (uint8_t*)attr + NLA_HDRLEN;
	int length = NLA_ALIGN(sizeof(value)) + NLA_HDRLEN;

	/* Validate parameters */
	if(attr == NULL || header == NULL) {
		return NULL;
	}

	/* Fill attribute buffer */
	attr->nla_len = length;
	attr->nla_type = type;
	memcpy(data, &value, sizeof(value));
	header->nlmsg_len += length;

	return msg;
}

/**
 * _qdl_put_msg_dynamic_attr
 * @msg: message buffer
 * @type: attribute type
 * @value: numeric value which needs to be added
 * @size: value size
 *
 * Fills message buffer with attribute of type dynamic.
 * Returns pointer to the message buffer if success, otherwise NULL.
 */
uint8_t* _qdl_put_msg_dynamic_attr(uint8_t *msg, uint16_t type, uint8_t *value, uint32_t size)
{
	struct nlmsghdr *header = (struct nlmsghdr*)msg;
	struct nlattr *attr = (struct nlattr*)_qdl_get_msg_tail(msg);
	uint8_t *data = (uint8_t*)attr + NLA_HDRLEN;
	int length = NLA_ALIGN(size) + NLA_HDRLEN;

	/* Validate parameters */
	if(attr == NULL || header == NULL || value == NULL || size == 0) {
		return NULL;
	}

	/* Fill attribute buffer */
	attr->nla_len = length;
	attr->nla_type = type;
	memcpy(data, value, size);
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
	header->nlmsg_seq = (uint64_t)time(NULL);		/* time_t is 64-bit wide from GCC 11, casting fixes Coverity hits */
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
	case NLMSG_NOOP:
	case NLMSG_DONE:
	case NLMSG_OVERRUN:
		break;
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
	struct nlattr *nattr = NULL;
	uint8_t *data = NULL;
	unsigned int i = 0;

	data = _qdl_get_attr_data_addr(msg, msg_size, (uint8_t*)attr);
	fprintf(fp, "-------- %11s ----\n", title);
	fprintf(fp, "len:     0x%04X (%d)\n", attr->nla_len, attr->nla_len);
	fprintf(fp, "type:    0x%04X\n", attr->nla_type);
	fprintf(fp, "value:   ");
	if(data != NULL) {
		if(_qdl_is_uint8_attr(attr, header->nlmsg_type)) {
			fprintf(fp, "%01X (%d)\n", *((uint8_t*)data), *((uint8_t*)data));
		} else if(_qdl_is_uint16_attr(attr, header->nlmsg_type)) {
			fprintf(fp, "%02X (%d)\n", *((uint16_t*)data), *((uint16_t*)data));
		} else if(_qdl_is_uint32_attr(attr, header->nlmsg_type)) {
			fprintf(fp, "%04X (%d)\n", *((uint32_t*)data), *((uint32_t*)data));
		} else if(_qdl_is_uint64_attr(attr, header->nlmsg_type)) {
			fprintf(fp, "%" PRIu64 " (%" PRIx64 ")\n", *((uint64_t*)data), *((uint64_t*)data));
		} else if(_qdl_is_string_attr(attr, header->nlmsg_type)) {
			fprintf(fp, "'%s'\n", (char*)data);
		} else if(_qdl_is_binary_attr(attr, header->nlmsg_type) ||
			  _qdl_is_dynamic_attr(attr, header->nlmsg_type)) {
			for(i = 0; i < attr->nla_len - 4; i++) {
				if(i != 0 && i % 16 == 0) {
					fprintf(fp, "\n         ");
				}
				fprintf(fp, "%02X ", data[i]);
			}
			fprintf(fp, "\n");
		} else if(_qdl_is_nattr(attr, header->nlmsg_type)) {
			fprintf(fp, "<nested>\n");
			nattr = (struct nlattr*)_qdl_get_next_nattr_addr((uint8_t*)attr, NULL);
			while(nattr != NULL) {
				_qdl_print_attr(fp, "nested attr", msg, msg_size, nattr);
				nattr = (struct nlattr*)_qdl_get_next_nattr_addr(
						(uint8_t*)attr, (uint8_t*)nattr);
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
void _qdl_print_msg(FILE *fp, uint8_t *buff, uint32_t buff_size)
{
	struct nlmsghdr *header = NULL;
	struct genlmsghdr *extra_header = NULL;
	struct nlattr *attr = NULL;
	uint8_t *msg = buff;
	uint32_t msg_size = 0;

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
			printf("Corrupted message (msg_size: %d, left_buff_size: %d)\n", msg_size, (uint32_t)(buff + buff_size - msg));
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
