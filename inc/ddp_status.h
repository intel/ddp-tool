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

#ifndef _DEF_DDP_STATUS_
#define _DEF_DDP_STATUS_

typedef uint16_t ddp_status_t;

/* when updating return code values please remember to update get_error_message() */
typedef enum _ddp_status_value_t{
    /* External error codes */
    DDP_SUCCESS = 0,
    DDP_BAD_COMMAND_LINE_PARAMETER,
    DDP_INTERNAL_GENERIC_ERROR,
    DDP_INSUFFICIENT_PRIVILEGES,
    DDP_NO_SUPPORTED_ADAPTER,
    DDP_NO_BASE_DRIVER,
    DDP_UNSUPPORTED_BASE_DRIVER,
    DDP_CANNOT_COMMUNICATE_ADAPTER,
    DDP_NO_DDP_PROFILE,
    DDP_CANNOT_READ_DEVICE_DATA,
    DDP_CANNOT_CREATE_OUTPUT_FILE,
    DDP_DEVICE_NOT_FOUND,
    /* Internal error codes */
    DDP_AQ_COMMAND_FAIL = 100,
    DDP_UNKNOWN_ETH_NAME,
    DDP_INCORRECT_BUFFER_SIZE,
    DDP_ALLOCATE_MEMORY_FAIL,
    DDP_INCORRECT_FUNCTION_PARAMETERS,
    DDP_NOT_IMPLEMENTED,
    DDP_ADAPTER_ERROR,
    DDP_CANNOT_CREATE_DEVICE_DESCRIPTOR
} ddp_status_value_t;

#endif
