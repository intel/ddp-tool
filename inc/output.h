/****************************************************************************************
* Copyright (C) 2019 - 2020 Intel Corporation
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
* OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* SPDX-License-Identifier: BSD-3-Clause
*
****************************************************************************************/

#ifndef _DDP_OUTPUT_H_
#define _DDP_OUTPUT_H_

#include "ddp.h"

bool
is_debug_print_enable();

void
debug_print_desc(adminq_desc_t* p_desc);

void
debug_print_drvinfo(driver_info_t* driver_info);

void
debug_print_ioctl(ioctl_structure_t* ioctl);

void
debug_ddp_print(char* format, ...);

void
ddp_print(char* format, ...);

/* Prototypes for ddp_func_print_adapter_list function pointer */

ddp_status_t
generate_table(list_t* adapter_list, ddp_status_value_t tool_status, char* file_name);

ddp_status_t
generate_xml(list_t* adapter_list, ddp_status_value_t tool_status, char* file_name);

ddp_status_t
generate_json(list_t* adapter_list, ddp_status_value_t tool_status, char* file_name);

/* Functions for printing adapter info for specific output */

void
print_table_adapter(adapter_t* adapter, FILE* stream);

void
print_xml_adapter(adapter_t* adapter, FILE* stream);

void
print_json_adapter(adapter_t* adapter, FILE* stream, uint32_t* number_of_nodes);

/* Error printing output functions */

ddp_status_t
generate_xml_error(ddp_status_value_t tool_status, char* file_name, char* error_message);

ddp_status_t
generate_json_error(ddp_status_value_t tool_status, char* file_name, char* error_message);

#endif
