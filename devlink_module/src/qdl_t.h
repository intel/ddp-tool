/*************************************************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2020 Intel Corporation.
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

#ifndef QDL_T_H_
#define QDL_T_H_

#include <linux/netlink.h>
#include <stdint.h>
#include <stdio.h>

#define QDL_BUS_NAME_LENGTH              16
#define QDL_PCI_LOCATION_NAME_LENGTH     32
#define QDL_FILE_NAME_MAX_LENGTH         256
#define QDL_REC_BUFF_SIZE                8192L   /* Buffer size for received message */
#define QDL_DRIVER_NET_INTERFACE_LENGTH  64

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
	int socket;
	struct sockaddr_nl socket_addr;
	char net_interface[QDL_DRIVER_NET_INTERFACE_LENGTH];
	uint32_t id;
	qdl_pci_t pci;
} qdl_struct;

typedef struct qdl_struct* qdl_dscr_t;
typedef int qdl_status_t;

/* Command ID */
enum {
	QDL_CMD_UNKNOWN,
	QDL_CMD_GET,

	QDL_CMD_PORT_GET = 5,

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

	QDL_DEVLINK_ATTR_REGION_NAME              = 88,   /* string */

	QDL_DEVLINK_ATTR_INFO_DRIVER_NAME         = 98,   /* string */
	QDL_DEVLINK_ATTR_INFO_SERIAL_NUMBER       = 99,   /* string */

	QDL_DEVLINK_ATTR_INFO_VERSION_FIXED       = 100,  /* nested */
	QDL_DEVLINK_ATTR_INFO_VERSION_RUNNING     = 101,  /* nested */
	QDL_DEVLINK_ATTR_INFO_VERSION_STORED      = 102,  /* nested */
	QDL_DEVLINK_ATTR_INFO_VERSION_NAME        = 103,  /* string */
	QDL_DEVLINK_ATTR_INFO_VERSION_VALUE       = 104,  /* string */

	QDL_DEVLINK_ATTR_FLASH_UPDATE_FILE_NAME   = 122,  /* string */

	QDL_DEVLINK_ATTR_ID_END
};

typedef enum {
	QDL_ATTR_TYPE_INVALID,
	QDL_ATTR_TYPE_UINT16,
	QDL_ATTR_TYPE_UINT32,
	QDL_ATTR_TYPE_STRING,
	QDL_ATTR_TYPE_NESTED,
	QDL_ATTR_TYPE_MCAST_GROUP
} qdl_attr_type_t;

#endif /* QDL_T_H_ */
