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

#ifndef _DEF_DDP_TYPES_
#define _DEF_DDP_TYPES_

#include <stdio.h>
#include <stdint.h>
#include <linux/types.h>

#include "ddp_status.h"

#define TRUE  1
#define FALSE 0

#define DDP_PROFILE_NAME_LENGTH               32
#define DDP_VERSION_LENGTH                    16
#define DDP_TRACKID_LENGTH                    9
#define DDP_TRACKID_SIZE                      8
#define DDP_DEVICE_INDEX_LENGTH               4
#define DDP_CONNECTION_NAME_NOT_AVAILABLE     "N/A"

typedef char     bool;
typedef uint64_t physical_address_t;
typedef struct   _node_t            node_t;

typedef enum _adapter_family_t{
    family_none = 0,
    family_40G,
    family_100G,
    family_last     /* add new entries before this one */
} adapter_family_t;

typedef struct _driver_os_version_t{
    uint16_t major;
    uint16_t minor;
    uint16_t build;
} driver_os_version_t;

typedef struct _driver_os_context_t{
    driver_os_version_t driver_version;
    bool                driver_available;
    bool                driver_supported;
} driver_os_context_t;

typedef struct _pci_config_space_t{
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t  reserved1[40];
    uint16_t subsystem_vendor_id;             /* PCI 2.2 spec */
    uint16_t subsystem_device_id;             /* PCI 2.2 spec */
    uint8_t  reserved2[16];
} pci_config_space_t;

typedef struct _driver_info_t{
    uint32_t  command;
    char      driver[32];             /* driver short name, "tulip", "eepro100" */
    char      version[32];            /* driver version string */
    char      firmware_version[32];   /* firmware version string, if applicable */
    char      bus_info[32];            /* Bus info for this IF. */
    char      reserved1[48];
    char      reserved2[16];
    uint32_t  net_stats;               /* number of u64's from ETHTOOL_GSTATS */
    uint32_t  test_info_length;
    uint32_t  eeprom_dump_length;       /* Size of data from ETHTOOL_GEEPROM (bytes) */
    uint32_t  register_dump_length;     /* Size of data from ETHTOOL_GREGS (bytes) */
} driver_info_t;

typedef struct _device_location_t{
    uint16_t segment;
    uint16_t bus;
    uint16_t device;
    uint16_t function;
} device_location_t;

typedef struct _nvm_version_t{
    uint8_t  nvm_version_major;
    uint8_t  nvm_version_minor;
} nvm_version_t;

typedef struct _firmware_version_t{
    uint32_t rom_version;
    uint32_t fw_build;
    uint16_t fw_major;
    uint16_t fw_minor;
    uint16_t fw_api_major;
    uint16_t fw_api_minor;
} firmware_version_t;

typedef struct _ioctl_structure_t{
    uint32_t command;
    uint32_t config;
    uint32_t offset;       /* in bytes */
    uint32_t data_size;    /* in bytes */
    uint8_t  data[1];      /* reserved memory for pointer for buffer */
} ioctl_structure_t;

typedef struct _adminq_desc_t{
    uint16_t flags;
    uint16_t opcode;
    uint16_t datalen;
    uint16_t retval;
    uint32_t cookie_high;
    uint32_t cookie_low;
    union{
        struct{
            uint32_t param0;
            uint32_t param1;
            uint32_t param2;
            uint32_t param3;
        } internal;
        struct{
            uint32_t param0;
            uint32_t param1;
            uint32_t addr_high;
            uint32_t addr_low;
        } external;
        uint8_t raw[16];
    } params;
} adminq_desc_t;

/* DDP PACKAGE */
typedef struct _ddp_profile_version_t{
    uint8_t major;
    uint8_t minor;
    uint8_t update;
    uint8_t draft;
} ddp_profile_version_t;

typedef struct _profile_info_t{
    uint32_t              section_size;
    uint32_t              track_id;
    ddp_profile_version_t version;
    uint8_t               owner;
    uint8_t               reserved[7];
    char                  name[DDP_PROFILE_NAME_LENGTH];
} profile_info_t;

typedef struct _adapter_t adapter_t;

/* Tool-Device Interface (TDI) definitions */
typedef ddp_status_t (*ddp_discovery_device)(adapter_t*);
typedef ddp_status_t (*ddp_check_fw_version)(adapter_t*, bool*, void*);
typedef bool (*ddp_is_virtual_function)(adapter_t*);

typedef struct _tool_device_interface{
    ddp_discovery_device    discovery_device;
    ddp_is_virtual_function is_virtual_function;
    ddp_check_fw_version    check_fw_version;
} ddp_tdi;

typedef struct _ddp_descriptor_t
{
    enum descriptor_type_t
    {
        descriptor_ioctl,
        descriptor_devlink,
        descriptor_none
    } descriptor_type;
    void* descriptor;
} ddp_descriptor_t;

struct _adapter_t{
    ddp_tdi            tdi;
    uint16_t           vendor_id;
    uint16_t           device_id;
    uint16_t           subvendor_id;
    uint16_t           subdevice_id;
    char*              branding_string;
    device_location_t  location;
    device_location_t  pf_location;
    char               connection_name[16];    /* the connection name presented in the OS, like eth0 */
    profile_info_t     profile_info;
    bool               is_virtual_function;
    bool               is_usable;              /* is usable to communicate with basedriver */
    char               pf_connection_name[16]; /* the connection name to PF used to read data for VF */
    uint16_t           pf_device_id;
    adapter_family_t   adapter_family;
};

typedef struct _node_t{
    void*     data;
    uint32_t  data_size;
    node_t*   next_node;
} node_t;

typedef struct _list_t{
    node_t*  first_node;
    uint32_t number_of_nodes;
} list_t;

typedef struct _supported_devices_t{
    uint16_t vendorid;
    uint16_t deviceid;
    uint16_t subvendorid;
    uint16_t subdeviceid;
    char*    branding_string;
} supported_devices_t;

typedef enum _adapter_parameter_t{
    adapter_none,
    adapter_location,
    adapter_interface
} adapter_parameter_t;

/* Types defining specific function pointers for generating output*/
typedef ddp_status_t (*ddp_output_function_t)(list_t*, ddp_status_value_t, char*);
ddp_output_function_t ddp_func_print_adapter_list;

#endif
