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

#ifndef _CMDPARAMS_H_
#define _CMDPARAMS_H_

#include "ddp.h"

#define DDP_CMD_LINE_MIN_SHORT_PARAMETER_SIZE       2
#define DDP_CMD_LINE_MIN_LONG_PARAMETER_SIZE        3
#define DDP_CMD_LINE_DOUBLE_DASH_PREFIX_SIZE        2

#define CONFLICT_PARAMETERS(a, b) (((static_command_line_parameters) & (a)) && \
                                   ((static_command_line_parameters) & (b))) ? \
                                  TRUE : FALSE

#define CHECK_DUPLICATE(a)        ((static_command_line_parameters & (a)) > 0) ? \
                                  DDP_BAD_COMMAND_LINE_PARAMETER : DDP_SUCCESS;

bool
check_command_parameter(uint32_t param);

ddp_status_t
parse_command_line_parameters(int argc, char ** argv, char ** interface_key, char ** file_name);

#endif
