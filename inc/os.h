/*************************************************************************************************************
* Copyright (C) 2019 Intel Corporation                                                                       *
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

#ifndef _DEF_OS_H
#define _DEF_OS_H

#include "ddp.h"

char*
strcat_sec(char* dest, uint32_t dest_size, const char* src, uint32_t chars_to_copy);

ddp_status_t
memcpy_sec(void* destination, uint32_t destination_size, void* source, uint32_t source_size);

ddp_status_t
strcpy_sec(char* destination, uint32_t destination_size, const char* source, uint32_t chars_to_copy);

char*
extended_strcpy_sec(char* dest, uint32_t dest_from, uint32_t dest_size, const char* src, uint32_t chars_to_copy);

void*
malloc_sec(size_t size);

bool
is_root_permission();

void
get_data_from_sysfs_files(adapter_t * adapter);

ddp_status_t
get_data_from_sysfs_config(adapter_t * adapter);

ddp_status_t
get_driver_version_from_os(driver_os_version_t* driver_version, char* version_file_path);

ddp_status_t
get_branding_string_via_pci_ids(adapter_t* ddp_adapter, match_level* match_level);

char*
replace_character(char* buffer, char find, char replace);

#endif
