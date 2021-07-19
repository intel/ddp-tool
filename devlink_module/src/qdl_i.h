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

#ifndef QDL_I_H_
#define QDL_I_H_

#include "qdl_t.h"
#include <stdlib.h>

qdl_status_t qdl_get_string_by_type(qdl_dscr_t dscr, uint8_t *buff, uint32_t buff_size, uint32_t type,
				    char *string, unsigned int string_size);
qdl_status_t qdl_get_string_by_key(qdl_dscr_t dscr, uint8_t *buff, uint32_t buff_size, char *name,
				   char *value, int value_size);
uint32_t qdl_get_region_header_size(uint32_t data_size);
qdl_status_t qdl_read_region(qdl_dscr_t dscr, uint8_t *msg_buff, uint32_t msg_buff_size, uint64_t offset,
			     uint8_t *bin_buff, unsigned int *bin_size);
qdl_status_t qdl_get_param_value(qdl_dscr_t dscr, uint8_t *buff, uint32_t buff_size, char *name,
				 qdl_param_cmode_t cmode, uint8_t *data, unsigned int *data_size);
qdl_status_t qdl_send_msg(qdl_dscr_t dscr, uint8_t *msg, unsigned int msg_size);
qdl_status_t qdl_receive_msg(qdl_dscr_t dscr, uint8_t *msg, unsigned int *msg_size);
qdl_status_t qdl_receive_reply_msg(qdl_dscr_t dscr, int cmd_type, void *data, uint8_t *reply_buff,
				   unsigned int *reply_buff_size);
uint8_t* qdl_create_msg(qdl_dscr_t dscr, int cmd_type, unsigned int *msg_size, void* data);
void qdl_release_dev(qdl_dscr_t qdl_dscr);
qdl_dscr_t qdl_init_dev(unsigned int segment, unsigned int bus, unsigned int device, unsigned int function,
			unsigned int flags);

#endif /* QDL_I_H_ */
