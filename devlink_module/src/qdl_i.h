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
