/****************************************************************************************
* Copyright (C) 2021 Intel Corporation
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
#ifndef _DEF_PACKAGE_FILE_H_
#define _DEF_PACKAGE_FILE_H_

#include "ddp_types.h"

#define DDP_SEGMENT_TYPE_GLOBAL_METADATA    0x0000000001
#define DDP_SEGMENT_TYPE_ICE_CONFIGURATION  0x0000000010
#define DDP_SEGMENT_TYPE_I40E_CONFIGURATION 0x0000000011

#define DDP_SECTION_TYPE_METADATA           0x0000000001

#define DDP_MAX_ICE_PROFILE_NAME_LENGTH     28
#define DDP_MAX_I40E_PROFILE_NAME_LENGTH    32
#define DDP_BUFFER_SIZE_4K                  4096

typedef enum _package_type_t{
    package_none,
    package_i40e,
    package_ice
} package_type_t;

#pragma pack(1)
typedef struct _package_header_t{
    uint32_t version;
    uint32_t entries_number;
    uint32_t segment_offset[0];
} package_header_t;

#pragma pack(1)
typedef struct _segment_header_t{
    uint32_t type;
    uint32_t version;
    uint32_t size;
    char     name[DDP_MAX_I40E_PROFILE_NAME_LENGTH];
    uint8_t  data[0];
} segment_header_t;

typedef struct _ice_buff_array_t{
    uint8_t buffer[DDP_BUFFER_SIZE_4K];
} ice_buff_array_t;
typedef struct _ice_buf_table_t{
    uint32_t buf_count;
    ice_buff_array_t buff_array[1];
} ice_buf_table_t;

typedef struct _ice_section_entry_t{
    uint32_t type;
    uint16_t offset;
    uint16_t size;
} ice_section_entry_t;

typedef struct _ice_buff_header_t{
    uint16_t section_count;
    uint16_t data_end;
    ice_section_entry_t section_entry[0];
} ice_buff_header_t;

typedef struct _ice_package_device_entry_t{
    uint16_t device_id;
    uint16_t subdevice_id;
} ice_package_device_entry_t;
typedef struct _ice_segment_header_t{
    segment_header_t generic_header;
    uint32_t device_table_count;
    ice_package_device_entry_t device[0];
} ice_segment_header_t;

typedef struct _ice_config_section_metadata{
    ddp_profile_version_t version;
    char      name[DDP_MAX_ICE_PROFILE_NAME_LENGTH];
    uint32_t  trackid;
} ice_config_section_metadata_t;

typedef struct _global_metadata_t{
    ddp_profile_version_t version;
    uint32_t trackid;
    char     name[DDP_MAX_ICE_PROFILE_NAME_LENGTH];
} global_metadata_t;

#pragma pack()

ddp_status_t
analyze_binary_file(list_t* adapter_list, char* input_file_name);

#endif /* _DEF_PACKAGE_FILE_H_ */