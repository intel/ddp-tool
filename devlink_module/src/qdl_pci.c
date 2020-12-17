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

#include "qdl_debug.h"
#include "qdl_codes.h"
#include "qdl_t.h"
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#define QDL_PCI_RESOURCES_DIR        "/sys/bus/pci/devices/"

/**
 * _qdl_get_pci_net_interface
 * @dscr: QDL descriptor
 * @buff: buffer for net interface name
 * @buff_size: buffer size
 *
 * Gets net interface name for specified devices.
 * Returns QDL_SUCCESS if success, otherwise an error code (QDL_NO_PCI_RESOURCES).
 */
qdl_status_t _qdl_get_pci_net_interface(qdl_dscr_t dscr, char *buff, unsigned int buff_size)
{
	char dir_name[QDL_FILE_NAME_MAX_LENGTH] = {'\0'};
	qdl_struct *dscr_data = (qdl_struct*)dscr;
	DIR *dir = NULL;
	struct dirent *dir_entry = NULL;
	qdl_status_t status = QDL_NO_PCI_RESOURCES;
	unsigned int length = 0;

	/* File name to access VPD data */
	sprintf(dir_name, "%s%04x:%02x:%02x.%x/net", QDL_PCI_RESOURCES_DIR, dscr_data->pci.seg,
		dscr_data->pci.bus, dscr_data->pci.dev, dscr_data->pci.fun);

	/* Open dir with PCI net resources */
	dir = opendir(dir_name);
	if(dir == NULL) {
		return QDL_NO_PCI_RESOURCES;
	}

	/* Read net interface name */
	dir_entry = readdir(dir);
	while(dir_entry != NULL) {
		if(strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0 ) {
			length = strlen(dir_entry->d_name);
			if(length < buff_size) {
				strncpy(buff, dir_entry->d_name, buff_size);
				status = QDL_SUCCESS;
			}
			break;
		}
		dir_entry = readdir(dir);
	}
	closedir(dir);

	return status;
}

/**
 * _qdl_read_pci_config_space
 * @dscr: QDL descriptor
 * @resources: resources which needs to be read
 * @buff: buffer for resources
 * @buff_size: buffer size
 *
 * Reads provided PCI resources (file) 'resources' from sysfs for specified device and copy it to 'buff'.
 * Returns QDL_SUCCESS if success, otherwise an error code (QDL_NO_PCI_RESOURCES).
 */
unsigned int _qdl_read_pci_resources(qdl_dscr_t dscr, char *resources, uint8_t *buff, unsigned int buff_size)
{
	char file_name[QDL_FILE_NAME_MAX_LENGTH] = { 0 };
	qdl_struct *dscr_data = (qdl_struct*)dscr;
	FILE *fp = NULL;
	int return_value = 0;
	unsigned int i = 0;
	unsigned int read_bytes = 0;

	/* File name to access PCI resources */
	sprintf(file_name, "%s%04x:%02x:%02x.%x/%s", QDL_PCI_RESOURCES_DIR, dscr_data->pci.seg,
		dscr_data->pci.bus, dscr_data->pci.dev, dscr_data->pci.fun, resources);

	/* Read PCI resources */
	fp = fopen(file_name, "r");
	if(fp == NULL) {
		QDL_DEBUGLOG_FUNCTION_FAIL("fopen", errno);
		return read_bytes;
	}
	for(i = 0; i < buff_size; i++) {
		return_value = fread(&buff[i], 1, 1, fp);
		if(return_value != 1) {
			break;
		}
	}
	read_bytes = i;
	fclose(fp);

	return read_bytes;
}

/**
 * _qdl_read_pci_config_space
 * @qdl_dscr: QDL descriptor
 *
 * Reads PCI Config Space data for specified device.
 * Returns number of read bytes.
 */
unsigned int _qdl_read_pci_config_space(qdl_dscr_t dscr)
{
	qdl_struct *dscr_data = (qdl_struct*)dscr;

	return _qdl_read_pci_resources(dscr, "config", (uint8_t*)&dscr_data->pci.config_space,
				       sizeof(qdl_pci_config_space_t));
}

/**
 * _qdl_read_pci_vpd
 * @qdl_dscr: QDL descriptor
 * @vpd_buff: buffer for VPD data
 * @vpd_buff_size: buffer size
 *
 * Reads VPD data for specified device.
 * Returns number of read bytes.
 */
unsigned int _qdl_read_pci_vpd(qdl_dscr_t dscr, uint8_t *vpd_buff, unsigned int vpd_buff_size)
{
	return _qdl_read_pci_resources(dscr, "vpd", vpd_buff, vpd_buff_size);
}

/**
 * _qdl_read_pci_mac_addr
 * @qdl_dscr: QDL descriptor
 * @mac_buff: buffer for MAC address string
 * @mac_buff_size: buffer size
 *
 * Reads MAC address string for specified device.
 * Returns number of read bytes.
 */
unsigned int _qdl_read_pci_mac_addr(qdl_dscr_t dscr, char *mac_buff, unsigned int mac_buff_size)
{
	char addr_string[QDL_FILE_NAME_MAX_LENGTH] = { 0 };
	qdl_struct *dscr_data = (qdl_struct*)dscr;

	/* File name to access MAC address */
	sprintf(addr_string, "net/%s/address", dscr_data->net_interface);

	return _qdl_read_pci_resources(dscr, addr_string, (uint8_t*)mac_buff, mac_buff_size);
}
