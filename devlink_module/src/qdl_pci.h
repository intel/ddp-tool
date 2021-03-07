/*************************************************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2020 - 2021 Intel Corporation.
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

#ifndef _QDL_PCI_H_
#define _QDL_PCI_H_

#include "qdl_t.h"

qdl_status_t _qdl_get_pci_net_interface(qdl_dscr_t dscr, char *buff, unsigned int buff_size);
unsigned int _qdl_read_pci_config_space(qdl_dscr_t dscr);
unsigned int _qdl_read_pci_vpd(qdl_dscr_t dscr, uint8_t *vpd_buff, unsigned int vpd_buff_size);
unsigned int _qdl_read_pci_mac_addr(qdl_dscr_t dscr, char *mac_buff, unsigned int mac_buff_size);

#endif /* _QDL_PCI_H_ */
