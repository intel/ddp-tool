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

#ifndef QDL_T_H_
#define QDL_T_H_

#include <linux/netlink.h>
#include <stdint.h>
#include <stdio.h>

/* Initialization flags */
#define QDL_INIT_NVM                     (1 << 0)
#define QDL_INIT_CAPS                    (1 << 1)

/* Message const */
#define QDL_REC_BUFF_SIZE                8192L   /* Buffer size for received message */
#define QDL_FLASH_CHUNK_MAX_SIZE         0x100   /* Chunk size */
#define QDL_NUM_OF_MSG_FLASH_CHUNKS      14      /* Number of flash chunks in a message */
#define QDL_FLASH_CHUNK_ATTR_SIZE        20
/* Headers size needed for one message with 14 chunks */
#define QDL_FLASH_CHUNK_HEADER_SIZE      (100 + QDL_NUM_OF_MSG_FLASH_CHUNKS * QDL_FLASH_CHUNK_ATTR_SIZE)

#define QDL_BUS_NAME_LENGTH              16
#define QDL_PCI_LOCATION_NAME_LENGTH     32
#define QDL_FILE_NAME_MAX_LENGTH         256
#define QDL_DRIVER_NET_INTERFACE_LENGTH  64
#define QDL_PARAM_NAME_SIZE              16
#define QDL_SREV_DATA_SIZE               4

/* Attributes names for "dev info" command */
#define QDL_INFO_NAME_FW_SECREV          "fw.mgmt.srev"
#define QDL_INFO_NAME_OROM_SECREV        "fw.undi.srev"

/* Parameter names */
#define QDL_FW_SREV_NAME                 "fw.mgmt.minsrev"
#define QDL_OROM_SREV_NAME               "fw.undi.minsrev"

#ifndef bool
typedef char bool;
#endif

#define true   1
#define false  0
typedef struct {
	uint16_t vendor_id;
	uint16_t device_id;
	uint16_t command;
	uint16_t status;
	uint8_t revision_id;
	uint8_t prog_if;
	uint8_t subclass_code;
	uint8_t class_code;
	uint8_t cache_ln;
	uint8_t lat_timer;
	uint8_t header;
	uint8_t bist;
	uint32_t bar0;
	uint32_t bar1;
	uint32_t bar2;
	uint32_t bar3;
	uint32_t bar4;
	uint32_t bar5;
	uint32_t cardbus_cis_pointer;
	uint16_t subsystem_vendor_id;
	uint16_t subsystem_id;
	uint32_t expansion_rom_base_address;
	uint8_t capabilities_pointer;
	uint8_t reserved[3];
	uint32_t reserved2;
	uint8_t interrupt_line;
	uint8_t interrupt_pin;
	uint8_t min_grant;
	uint8_t max_latency;
} qdl_pci_config_space_t;

typedef struct {
	uint16_t seg;
	uint8_t bus;
	uint8_t dev;
	uint8_t fun;
	qdl_pci_config_space_t config_space;
} qdl_pci_t;

typedef struct {
	char* name;                                           /* Region name */
	uint32_t snapshot_id;                                 /* Snapshot ID */
	bool new_snapshot_id;                                 /* did QDL create snapshot ID */
} qdl_region_t;

typedef struct {
	int socket;                                           /* socked descriptor */
	struct sockaddr_nl socket_addr;                       /* socked address */
	char net_interface[QDL_DRIVER_NET_INTERFACE_LENGTH];  /* interface name for device */
	uint32_t id;                                          /* message type */
	qdl_region_t flash_region;                            /* flash region description */
	qdl_region_t caps_region;                             /* caps region description */
	qdl_pci_t pci;
} qdl_struct;

typedef struct qdl_struct* qdl_dscr_t;
typedef int qdl_status_t;

/* Command ID */
enum {
	QDL_CMD_UNKNOWN,
	QDL_CMD_GET,

	QDL_CMD_PORT_GET = 5,

	QDL_CMD_RELOAD = 37,

	QDL_CMD_PARAM_GET = 38,
	QDL_CMD_PARAM_SET = 39,

	QDL_CMD_REGION_GET = 42,
	QDL_CMD_REGION_SET = 43,
	QDL_CMD_REGION_NEW = 44,
	QDL_CMD_REGION_DEL = 45,
	QDL_CMD_REGION_READ = 46,

	QDL_CMD_INFO_GET = 51,

	QDL_CMD_FLASH_UPDATE = 58,
	QDL_CMD_FLASH_UPDATE_END = 59,
	QDL_CMD_FLASH_UPDATE_STATUS = 60
};

/* Devlink attribute ID */
enum {
	QDL_DEVLINK_ATTR_UNKNOWN,
	QDL_DEVLINK_ATTR_BUS_NAME,                        /* string */
	QDL_DEVLINK_ATTR_LOCATION,                        /* string */
	QDL_DEVLINK_ATTR_PORT_INDEX,                      /* uint32 */
	QDL_DEVLINK_ATTR_PORT_TYPE,                       /* uint16 */
	QDL_DEVLINK_ATTR_DESIRED_TYPE,                    /* uint16 */
	QDL_DEVLINK_ATTR_NETDEV_IF_INDEX,                 /* uint32 */
	QDL_DEVLINK_ATTR_NETDEV_NAME,                     /* string */

	QDL_DEVLINK_ATTR_PORT_FLAVOUR             = 77,   /* uint16 */
	QDL_DEVLINK_ATTR_PORT_NUMBER              = 78,   /* uint32 */

	QDL_DEVLINK_ATTR_PARAM                    = 80,   /* nested */
	QDL_DEVLINK_ATTR_PARAM_NAME               = 81,   /* string */
	QDL_DEVLINK_ATTR_PARAM_GENERIC            = 82,   /* flag */
	QDL_DEVLINK_ATTR_PARAM_TYPE               = 83,   /* uint8  */
	QDL_DEVLINK_ATTR_PARAM_VALUES_LIST        = 84,   /* nested */
	QDL_DEVLINK_ATTR_PARAM_VALUE              = 85,   /* nested */
	QDL_DEVLINK_ATTR_PARAM_VALUE_DATA         = 86,   /* dynamic */
	QDL_DEVLINK_ATTR_PARAM_VALUE_CMODE        = 87,   /* uint8 */

	QDL_DEVLINK_ATTR_REGION_NAME              = 88,   /* string */
	QDL_DEVLINK_ATTR_REGION_SIZE              = 89,   /* uint64 */
	QDL_DEVLINK_ATTR_REGION_SNAPSHOTS         = 90,   /* nested */
	QDL_DEVLINK_ATTR_REGION_SNAPSHOT          = 91,   /* nested */
	QDL_DEVLINK_ATTR_REGION_SNAPSHOT_ID       = 92,   /* uint32 */
	QDL_DEVLINK_ATTR_REGION_CHUNKS            = 93,   /* nested */
	QDL_DEVLINK_ATTR_REGION_CHUNK             = 94,   /* nested */
	QDL_DEVLINK_ATTR_REGION_CHUNK_DATA        = 95,   /* binary */
	QDL_DEVLINK_ATTR_REGION_CHUNK_ADDR        = 96,   /* uint64 */
	QDL_DEVLINK_ATTR_REGION_CHUNK_LEN         = 97,   /* uint64 */

	QDL_DEVLINK_ATTR_INFO_DRIVER_NAME         = 98,   /* string */
	QDL_DEVLINK_ATTR_INFO_SERIAL_NUMBER       = 99,   /* string */
	QDL_DEVLINK_ATTR_INFO_VERSION_FIXED       = 100,  /* nested */
	QDL_DEVLINK_ATTR_INFO_VERSION_RUNNING     = 101,  /* nested */
	QDL_DEVLINK_ATTR_INFO_VERSION_STORED      = 102,  /* nested */
	QDL_DEVLINK_ATTR_INFO_VERSION_NAME        = 103,  /* string */
	QDL_DEVLINK_ATTR_INFO_VERSION_VALUE       = 104,  /* string */

	QDL_DEVLINK_ATTR_FLASH_UPDATE_FILE_NAME   = 122,  /* string */

	QDL_DEVLINK_ATTR_RELOAD_ACTION            = 153,  /* uint8 */

	QDL_DEVLINK_ATTR_ID_END
};

/* Attribute type */
typedef enum {
	QDL_ATTR_TYPE_INVALID,
	QDL_ATTR_TYPE_UINT8,
	QDL_ATTR_TYPE_UINT16,
	QDL_ATTR_TYPE_UINT32,
	QDL_ATTR_TYPE_UINT64,
	QDL_ATTR_TYPE_STRING,
	QDL_ATTR_TYPE_FLAG,
	QDL_ATTR_TYPE_NESTED  = 8,
	QDL_ATTR_TYPE_BINARY  = 11,
	QDL_ATTR_TYPE_DYNAMIC,
	QDL_ATTR_TYPE_MCAST_GROUP
} qdl_attr_type_t;

/* Param cmode values */
typedef enum {
	QDL_PARAM_CMODE_RUNTIME,
	QDL_PARAM_CMODE_DRIVERINIT,
	QDL_PARAM_CMODE_PERMANENT,
} qdl_param_cmode_t;

typedef enum {
	QDL_PARAM_RELOAD_ACTION_UNSPEC,
	QDL_PARAM_RELOAD_ACTION_DRIVER_REINIT,
	QDL_PARAM_RELOAD_ACTION_FW_ACTIVATE
} qdl_param_reload_action_t;

/* Additional data for messages */
typedef struct {
	char* region;                     /* Region name */
	uint64_t address;                 /* Address of the region to read */
	uint64_t length;                  /* Length of the region to read */
} qdl_msg_region_read_t;

typedef struct {
	char* minsrev_name;               /* MinSrev name */
	uint8_t minsrev_value;            /* MinSrev value */
} qdl_msg_param_set_t;

#endif /* QDL_T_H_ */
