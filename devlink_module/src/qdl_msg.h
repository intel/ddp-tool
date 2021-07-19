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

#ifndef _QDL_MSG_H_
#define _QDL_MSG_H_

#include "qdl_t.h"

#define QDL_REGION_NAME_FLASH  "nvm-flash"
#define QDL_REGION_NAME_CAPS   "device-caps"
#define QDL_REGION_NAME_SIZE   12                /* Max length of the region name */

bool _qdl_is_ctrl_msg(struct nlmsghdr *msg);
uint8_t* _qdl_get_msg_data_addr(uint8_t *msg);
qdl_status_t _qdl_validate_region_name(char* name);
int _qdl_get_msg_size(int cmd_type);
uint8_t* _qdl_get_next_msg(uint8_t *buff, unsigned int buff_size, uint8_t *msg);
qdl_status_t _qdl_get_uint32_attr(uint8_t *msg, uint32_t msg_size, uint32_t type, uint32_t *value);
qdl_status_t _qdl_get_string_attr(uint8_t *msg, uint32_t msg_size, uint32_t type, char *string,
		unsigned int string_size);
qdl_status_t _qdl_get_string_nattr_by_key(uint8_t *msg, uint32_t msg_size, char *name, char *value,
		unsigned int value_size);
qdl_status_t _qdl_get_int_nattr_by_type(uint8_t *msg, uint32_t msg_size, uint32_t type, uint32_t *value);
qdl_status_t _qdl_get_region(uint8_t *msg, uint32_t msg_size, uint8_t *bin_buff, unsigned int *bin_size,
			     uint64_t *init_offset);
qdl_status_t _qdl_get_param_value(uint8_t *msg, uint32_t msg_size, uint8_t cmode, uint8_t *data,
				  unsigned int *data_size);
uint8_t* _qdl_put_msg_extra_header(uint8_t *msg, uint8_t cmd, uint8_t version);
uint8_t* _qdl_put_msg_header(uint8_t *msg, uint16_t type, uint16_t flags);
uint8_t* _qdl_put_msg_str_attr(uint8_t *msg, uint16_t type, char *string);
uint8_t* _qdl_put_msg_uint8_attr(uint8_t *msg, uint16_t type, uint8_t value);
uint8_t* _qdl_put_msg_uint32_attr(uint8_t *msg, uint16_t type, uint32_t value);
uint8_t* _qdl_put_msg_uint64_attr(uint8_t *msg, uint16_t type, uint64_t value);
uint8_t* _qdl_put_msg_dynamic_attr(uint8_t *msg, uint16_t type, uint8_t *value, uint32_t size);
uint8_t* _qdl_create_generic_msg(int id, int cmd_type, unsigned int *msg_size);
void _qdl_print_msg(FILE *fp, uint8_t *buff, unsigned int buff_size);

#endif /* _QDL_MSG_H_ */
