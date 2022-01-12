/****************************************************************************************
* Copyright (C) 2019 - 2021 Intel Corporation
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

#ifndef _DEF_DDP_H_
#define _DEF_DDP_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include <linux/types.h>
#include <sys/stat.h>

#define upper_16_bits(x) ((uint16_t)((x) >> 16))
#define lower_16_bits(x) ((uint16_t)(x))

#include <linux/errno.h>
#include <linux/pci.h>

#include "ddp_types.h"
#include "ddp_status.h"
#include "ddp_list.h"
#include "output.h"
#include "os.h"
#include "i40e.h"
#include "ice.h"

#include <unistd.h>

#define PATH_TO_SYSFS_PCI   "/sys/bus/pci/devices/"
#define PATH_TO_PCI_DRIVERS "/sys/bus/pci/drivers/"

#define ETHTOOL_GDRVINFO 0x00000003
#define ETHTOOL_IOCTL    0x8946

#define BASEDRIVER_GET_DRIVER_INFO          3
#define BASEDRIVER_READNVM_FUNCID           11
#define BASEDRIVER_WRITENVM_FUNCID          12

#define DDP_DECIMAL_SYSTEM                  10
#define DDP_HEXADECIMAL_SYSTEM              16
#define DDP_DWORD_LENGTH                    4

#define PCI_LOCATION_STRING_SIZE            12

#define MEMINIT(x) memset(x, 0, sizeof(*x))
#define MEMINIT_ARRAY(x,y) memset((x), 0, (y)*sizeof(*(x)))

typedef struct ifreq ifreq_t;

#define DDP_DRIVER_NAME_40G               "i40e"
#define DDP_DRIVER_NAME_100G              "ice"

/* String buffer values */
#define MAX_DRIVER_VERSION_STRING_LENTGH  32
#define NUMBER_OF_DRIVER_VERSION_PARTS    3

#define DDP_MAX_BUFFER_SIZE               1024
#define DDP_MAX_NAME_LENGTH               128
#define DDP_MAX_BRANDING_SIZE             2048
#define DDP_ID_BUFFER_SIZE                5     /* 4-part ID (4 chars) + null terminator */

#define PCI_DEVICE_CONFIG_DWORDS          16

/* Admin Queue defaults */
#define I40E_DEFAULT_DESCRIPTORS          32
#define I40E_ADMINQ_DEFAULT_BUFFER_SIZE   8

#define NUMBER_OF_PROFILES_LENGTH         4
#define OFFSET_TO_ADMINQ_WRITEBACK        16
#define OFFSET_TO_PROFILE_LIST_SIZE       16
#define DDP_ADMINQ_WRITEBACK_SIZE         0x500
#define IOCTL_EXECUTE_COMMAND             0xF << 8
#define IOCTL_REGISTER_ACCESS_COMMAND     0x0100

#define DDP_MAJOR_VERSION                   1
#define DDP_MINOR_VERSION                   0
#define DDP_BUILD_VERSION                   14
#define DDP_FIX_VERSION                     0

#define DDP_MIN_BASE_DRIVER_VERSION_MAJOR 2
#define DDP_MIN_BASE_DRIVER_VERSION_MINOR 7
#define DDP_MIN_BASE_DRIVER_VERSION_BUILD 27

#define MAX_FILE_NAME                     300

#define DDP_LOCATION_COMMAND_PARAMETER    's'
#define DDP_HELP1_COMMAND_PARAMETER       'h'
#define DDP_HELP2_COMMAND_PARAMETER       '?'
#define DDP_INTERFACE_COMMAND_PARAMETER   'i'
#define DDP_XML_COMMAND_PARAMETER         'x'
#define DDP_VERSION_COMMAND_PARAMETER     'v'
#define DDP_SILENT_MODE                   'l'
#define DDP_JSON_COMMAND_PARAMETER        'j'
#define DDP_ALL_ADAPTERS_PARAMETER        'a'

#define DDP_LOCATION_COMMAND_PARAMETER_BIT  (1 << 0)  /* '-s' - location command line parameter */
#define DDP_ALL_ADAPTERS_PARAMETER_BIT      (1 << 1)  /* '-a' - Show information about all functions supported devices */
#define DDP_HELP_COMMAND_PARAMETER_BIT      (1 << 2)  /* '-h', '-?', '--help' - help command line parameter */
#define DDP_INTERFACE_COMMAND_PARAMETER_BIT (1 << 3)  /* '-i' - interface command line parameter */
#define DDP_XML_COMMAND_PARAMETER_BIT       (1 << 4)  /* '-x' - XML file command line parameter */
#define DDP_VERSION_COMMAND_PARAMETER_BIT   (1 << 5)  /* '-v' - version command line parameter */
#define DDP_SILENT_MODE_PARAMETER_BIT       (1 << 6)  /* '-l' - silent mode for scripts*/
#define DDP_JSON_COMMAND_PARAMETER_BIT      (1 << 7)  /* '-j' - JSON file command line parameter */

#define COMPARE_PCI_LOCATION(a, b) ((a)->location.segment) == ((b)->location.segment) ? \
                                    ((a)->location.bus) == ((b)->location.bus) ? TRUE : FALSE : FALSE

/* Messages dictionary */
#define EMPTY_MESSAGE  "-"
#define NO_PROFILE     "No profile loaded"
#define UNSUPPORTED_FW "Unsupported FW version"

void
free_memory(void* pointer);

bool
is_device_supported(adapter_t* adapter);

ddp_status_t
get_nvm_version(adapter_t* adapter, nvm_version_t* nvm_version);

adapter_t*
get_adapter_from_list_node(node_t* node);

char*
get_error_message(ddp_status_value_t status);

ddp_status_t
execute_adminq_command(adapter_t* adapter, adminq_desc_t* descriptor, uint16_t descriptor_size, uint8_t* cmd_buffer);

ddp_status_t
write_register(adapter_t* adapter, uint32_t reg_address, uint32_t byte_number, void* input_register);

ddp_status_t
read_register(adapter_t* adapter, uint32_t reg_address, uint32_t byte_number, void* output_register);

void
free_ddp_adapter_list_allocated_fields(list_t* adapter_list);

#endif
