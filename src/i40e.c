/****************************************************************************************
* Copyright (C) 2020 Intel Corporation
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

#include "i40e.h"
#include "ddp.h"

supported_devices_t i40e_supported_devices[] =
{
    {0x8086, 0x0CF8, 0x8086, 0x0000, "Intel(R) Ethernet Controller X710 Intel(R) FPGA Programmable Acceleration Card N3000 for Networking"},
    {0x8086, 0x0CF8, 0x8086, 0x0001, "Intel(R) Ethernet Controller X710 Intel(R) FPGA Programmable Acceleration Card N3000 for Networking"},
    {0x8086, 0x0D58, 0x8086, 0x0000, "Intel(R) Ethernet Controller XXV710 Intel(R) FPGA Programmable Acceleration Card N3000 for Networking"},
    {0x8086, 0x0D58, 0x8086, 0x0001, "Intel(R) Ethernet Controller XXV710 Intel(R) FPGA Programmable Acceleration Card N3000 for Networking"},
    {0x8086, 0x101F, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller V710 for 5GBASE-T"},
    {0x8086, 0x104E, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller X710 for 10 Gigabit SFP+"},
    {0x8086, 0x104F, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller X710 for 10 Gigabit backplane"},
    {0x8086, 0x154B, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller XL710 Generic ID"},
    {0x8086, 0x154C, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Virtual Function 700 Series"},
    {0x8086, 0x1572, 0x1028, 0x0000, "Intel(R) Ethernet 10G X710 rNDC"},
    {0x8086, 0x1572, 0x1028, 0x1F99, "Intel(R) Ethernet 10G 4P X710/I350 rNDC"},
    {0x8086, 0x1572, 0x1028, 0x1F9C, "Intel(R) Ethernet 10G 4P X710 SFP+ rNDC"},
    {0x8086, 0x1572, 0x103C, 0x0000, "HPE Ethernet 10Gb 562SFP+ Adapter"},
    {0x8086, 0x1572, 0x103C, 0x22FC, "HPE Ethernet 10Gb 2-port 562FLR-SFP+ Adapter"},
    {0x8086, 0x1572, 0x103C, 0x22FD, "HPE Ethernet 10Gb 2-port 562SFP+ Adapter"},
    {0x8086, 0x1572, 0x1137, 0x0000, "Cisco(R) Ethernet Converged NIC X710-DA"},
    {0x8086, 0x1572, 0x1137, 0x013B, "Cisco(R) Ethernet Converged NIC X710-DA4"},
    {0x8086, 0x1572, 0x1137, 0x020A, "Cisco(R) Ethernet Converged NIC X710-DA2"},
    {0x8086, 0x1572, 0x1590, 0x0000, "Intel(R) Ethernet Controller X710 for 10GbE SFP+"},
    {0x8086, 0x1572, 0x1590, 0x0225, "HPE Eth 10Gb 4p 563SFP+ Adptr"},
    {0x8086, 0x1572, 0x1590, 0x022F, "HPE Ethernet 10Gb 2-port 564i Communication Board"},
    {0x8086, 0x1572, 0x17AA, 0x0000, "Lenovo ThinkServer X710 AnyFabric for 10Gbe SFP+"},
    {0x8086, 0x1572, 0x17AA, 0x4001, "Lenovo ThinkServer X710-4 AnyFabric for 10Gbe SFP+"},
    {0x8086, 0x1572, 0x17AA, 0x4002, "Lenovo ThinkServer X710-2 AnyFabric for 10Gbe SFP+"},
    {0x8086, 0x1572, 0x8086, 0x0000, "Intel(R) Ethernet Converged Network Adapter X710"},
    {0x8086, 0x1572, 0x8086, 0x0001, "Intel(R) Ethernet Converged Network Adapter X710-4"},
    {0x8086, 0x1572, 0x8086, 0x0002, "Intel(R) Ethernet Converged Network Adapter X710-4"},
    {0x8086, 0x1572, 0x8086, 0x0004, "Intel(R) Ethernet Converged Network Adapter X710-4"},
    {0x8086, 0x1572, 0x8086, 0x0005, "Intel(R) Ethernet Converged Network Adapter X710"},
    {0x8086, 0x1572, 0x8086, 0x0006, "Intel(R) Ethernet Converged Network Adapter X710"},
    {0x8086, 0x1572, 0x8086, 0x0007, "Intel(R) Ethernet Converged Network Adapter X710-2"},
    {0x8086, 0x1572, 0x8086, 0x0008, "Intel(R) Ethernet Converged Network Adapter X710-2"},
    {0x8086, 0x1572, 0x8086, 0x0009, "Intel(R) Ethernet Controller X710 for 10GbE SFP+"},
    {0x8086, 0x1572, 0x8086, 0x000A, "Intel(R) Ethernet Controller X710 for 10GbE SFP+"},
    {0x8086, 0x1572, 0x8086, 0x000B, "Intel(R) Ethernet Server Adapter X710-DA2 for OCP"},
    {0x8086, 0x1572, 0x8086, 0x000D, "Intel(R) Ethernet Controller X710 for 10GbE SFP+"},
    {0x8086, 0x1572, 0x8086, 0x000E, "Intel(R) Ethernet Server Adapter OCP X710-2"},
    {0x8086, 0x1572, 0x8086, 0x000F, "Intel(R) Ethernet Server Adapter OCP X710-2"},
    {0x8086, 0x1572, 0x8086, 0x0013, "Intel(R) Ethernet 10G 2P X710 OCP"},
    {0x8086, 0x1572, 0x8086, 0x0014, "Intel(R) Ethernet 10G 4P X710 OCP"},
    {0x8086, 0x1572, 0x8086, 0x0015, "Intel(R) Ethernet Server Adapter X710-DA2 for OCP"},
    {0x8086, 0x1572, 0x8086, 0x4005, "Intel(R) Ethernet Controller X710 for 10GbE SFP+"},
    {0x8086, 0x1572, 0x8086, 0x4006, "Intel(R) Ethernet Controller X710 for 10GbE SFP+"},
    {0x8086, 0x1572, 0x8086, 0x4007, "Intel(R) Ethernet Controller X710 for 10GbE SFP+"},
    {0x8086, 0x1572, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller X710 for 10GbE SFP+"},
    {0x8086, 0x1573, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller X710 for 10GbE SFP+"},
    {0x8086, 0x1574, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller XL710 QEMU"},
    {0x8086, 0x1580, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller XL710 for 40GbE backplane"},
    {0x8086, 0x1581, 0x1028, 0x0000, "Intel(R) Ethernet 10G X710-k bNDC"},
    {0x8086, 0x1581, 0x1028, 0x1F98, "Intel(R) Ethernet 10G 4P X710-k bNDC"},
    {0x8086, 0x1581, 0x1028, 0x1F9E, "Intel(R) Ethernet 10G 2P X710-k bNDC"},
    {0x8086, 0x1581, 0x1590, 0x00F8, "HPE Ethernet 10Gb 2-port 563i Adapter"},
    {0x8086, 0x1581, 0x1590, 0x0000, "HPE Ethernet 10Gb 2-port 563i Adapter"},
    {0x8086, 0x1581, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller X710 for 10GbE backplane"},
    {0x8086, 0x1582, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller X710 for 10GbE backplane"},
    {0x8086, 0x1583, 0x1028, 0x0000, "Intel(R) Ethernet 40G 2P XL710 QSFP+ rNDC"},
    {0x8086, 0x1583, 0x1028, 0x1F9F, "Intel(R) Ethernet 40G 2P XL710 QSFP+ rNDC"},
    {0x8086, 0x1583, 0x108E, 0x0000, "Oracle 10 Gb/40 Gb Ethernet Adapter"},
    {0x8086, 0x1583, 0x108E, 0x7B1B, "Oracle Quad 10Gb Ethernet Adapter"},
    {0x8086, 0x1583, 0x108E, 0x7B1D, "Oracle 10 Gb/40 Gb Ethernet Adapter"},
    {0x8086, 0x1583, 0x1137, 0x0000, "Cisco(R) Ethernet Converged NIC XL710-QDA2"},
    {0x8086, 0x1583, 0x1137, 0x013C, "Cisco(R) Ethernet Converged NIC XL710-QDA2"},
    {0x8086, 0x1583, 0x8086, 0x0000, "Intel(R) Ethernet Converged Network Adapter XL710-Q2"},
    {0x8086, 0x1583, 0x8086, 0x0001, "Intel(R) Ethernet Converged Network Adapter XL710-Q2"},
    {0x8086, 0x1583, 0x8086, 0x0002, "Intel(R) Ethernet Converged Network Adapter XL710-Q2"},
    {0x8086, 0x1583, 0x8086, 0x0003, "Intel(R) Ethernet I/O Module XL710-Q2"},
    {0x8086, 0x1583, 0x8086, 0x0004, "Intel(R) Ethernet Server Adapter XL710-Q2OCP"},
    {0x8086, 0x1583, 0x8086, 0x0006, "Intel(R) Ethernet Converged Network Adapter XL710-Q2"},
    {0x8086, 0x1583, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller XL710 for 40GbE QSFP+"},
    {0x8086, 0x1584, 0x8086, 0x0000, "Intel(R) Ethernet Converged Network Adapter XL710-Q1"},
    {0x8086, 0x1584, 0x8086, 0x0001, "Intel(R) Ethernet Converged Network Adapter XL710-Q1"},
    {0x8086, 0x1584, 0x8086, 0x0002, "Intel(R) Ethernet Converged Network Adapter XL710-Q1"},
    {0x8086, 0x1584, 0x8086, 0x0003, "Intel(R) Ethernet I/O Module XL710-Q1"},
    {0x8086, 0x1584, 0x8086, 0x0004, "Intel(R) Ethernet Server Adapter XL710-Q1OCP"},
    {0x8086, 0x1584, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller XL710 for 40GbE QSFP+"},
    {0x8086, 0x1585, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller XL710 for 10GbE QSFP+"},
    {0x8086, 0x1586, 0x108E, 0x0000, "Intel(R) Ethernet Controller X710 for 10GBASE-T"},
    {0x8086, 0x1586, 0x108E, 0x4857, "Intel(R) Ethernet Controller X710 for 10GBASE-T"},
    {0x8086, 0x1586, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller X710 for 10GBASE-T"},
    {0x8086, 0x1587, 0x103C, 0x0000, "HPE Eth 10/20Gb 2p 660FLB Adptr"},
    {0x8086, 0x1587, 0x103C, 0x22FE, "HPE Eth 10/20Gb 2p 660FLB Adptr"},
    {0x8086, 0x1588, 0x103C, 0x0000, "HPE Eth 10/20Gb 2p 660M Adptr"},
    {0x8086, 0x1588, 0x103C, 0x22FE, "HPE Flex-20 20Gb 2-port 660M Adapter"},
    {0x8086, 0x1588, 0x103C, 0x22FF, "HPE Eth 10/20Gb 2p 660M Adptr"},
    {0x8086, 0x1589, 0x108E, 0x0000, "Oracle Quad Port 10GBase-T Adapter"},
    {0x8086, 0x1589, 0x108E, 0x7B1C, "Oracle Quad Port 10GBase-T Adapter"},
    {0x8086, 0x1589, 0x1137, 0x0000, "Cisco(R) Ethernet Converged NIC X710-T4"},
    {0x8086, 0x1589, 0x1137, 0x020B, "Cisco(R) Ethernet Converged NIC X710-T4"},
    {0x8086, 0x1589, 0x8086, 0x0000, "Intel(R) Ethernet Converged Network Adapter X710-T"},
    {0x8086, 0x1589, 0x8086, 0x0001, "Intel(R) Ethernet Converged Network Adapter X710-T4"},
    {0x8086, 0x1589, 0x8086, 0x0002, "Intel(R) Ethernet Converged Network Adapter X710-T4"},
    {0x8086, 0x1589, 0x8086, 0x0003, "Intel(R) Ethernet Converged Network Adapter X710-T"},
    {0x8086, 0x1589, 0x8086, 0x00A0, "Intel(R) Ethernet Converged Network Adapter X710-T4"},
    {0x8086, 0x1589, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller X710/X557-AT 10GBASE-T"},
    {0x8086, 0x158A, 0x1590, 0x0000, "HPE 10/25Gb Ethernet Adapter"},
    {0x8086, 0x158A, 0x1590, 0x0286, "HPE Synergy 4610C 10/25Gb Ethernet Adapter"},
    {0x8086, 0x158A, 0x8086, 0x0000, "Intel(R) Ethernet Controller XXV710 for 25GbE backplane"},
    {0x8086, 0x158A, 0x8086, 0x000A, "Intel(R) Ethernet 25G 2P XXV710 Mezz"},
    {0x8086, 0x158A, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller XXV710 for 25GbE backplane"},
    {0x8086, 0x158B, 0x1137, 0x0000, "Cisco(R) Ethernet Network Adapter XXV710"},
    {0x8086, 0x158B, 0x1137, 0x0225, "Cisco(R) Ethernet Network Adapter XXV710"},
    {0x8086, 0x158B, 0x1590, 0x0000, "Intel(R) Ethernet Network Adapter XXV710-2"},
    {0x8086, 0x158B, 0x1590, 0x0253, "HPE Ethernet 10/25Gb 2-port 661SFP28 Adapter"},
    {0x8086, 0x158B, 0x8086, 0x0000, "Intel(R) Ethernet Network Adapter XXV710"},
    {0x8086, 0x158B, 0x8086, 0x0001, "Intel(R) Ethernet Network Adapter XXV710-2"},
    {0x8086, 0x158B, 0x8086, 0x0002, "Intel(R) Ethernet Network Adapter XXV710-2"},
    {0x8086, 0x158B, 0x8086, 0x0003, "Intel(R) Ethernet Network Adapter XXV710-1"},
    {0x8086, 0x158B, 0x8086, 0x0004, "Intel(R) Ethernet Network Adapter XXV710-1"},
    {0x8086, 0x158B, 0x8086, 0x0005, "Intel(R) Ethernet Network Adapter OCP XXV710-2"},
    {0x8086, 0x158B, 0x8086, 0x0006, "Intel(R) Ethernet Network Adapter OCP XXV710-2"},
    {0x8086, 0x158B, 0x8086, 0x0007, "Intel(R) Ethernet Network Adapter OCP XXV710-1"},
    {0x8086, 0x158B, 0x8086, 0x0008, "Intel(R) Ethernet Network Adapter OCP XXV710-1"},
    {0x8086, 0x158B, 0x8086, 0x0009, "Intel(R) Ethernet 25G 2P XXV710 Adapter"},
    {0x8086, 0x158B, 0x8086, 0x000A, "Intel(R) Ethernet 25G 2P XXV710 OCP"},
    {0x8086, 0x158B, 0x8086, 0x4001, "Intel(R) Ethernet Network Adapter XXV710-2"},
    {0x8086, 0x158B, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller XXV710 for 25GbE SFP28"},
    {0x8086, 0x15FF, 0x8086, 0x0000, "Intel(R) Ethernet Network Adapter X710-TL"},
    {0x8086, 0x15FF, 0x8086, 0x0001, "Intel(R) Ethernet Network Adapter X710-T4L"},
    {0x8086, 0x15FF, 0x8086, 0x0002, "Intel(R) Ethernet Network Adapter X710-T4L"},
    {0x8086, 0x15FF, 0x8086, 0x0003, "Intel(R) Ethernet Network Adapter X710-T2L"},
    {0x8086, 0x15FF, 0x8086, 0x0004, "Intel(R) Ethernet Network Adapter X710-T2L"},
    {0x8086, 0x15FF, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller X710 for 10GBASE-T"},
    {0x8086, 0xFAFA, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller XL710 Barleyville"},
    {0x8086, 0xFBFB, 0xFFFF, 0xFFFF, "Intel(R) XL710 X710 Virtual Function Barleyville"},
};

uint16_t i40e_supported_devices_size = sizeof(i40e_supported_devices)/sizeof(supported_devices_t);

ddp_status_t
_i40e_check_fw_version(adapter_t* adapter, bool* is_fw_supported)
{
    ddp_status_t  status = DDP_SUCCESS;
    nvm_version_t nvm_version;

    MEMINIT(&nvm_version);

    do
    {
        /* For FVL devices the tool has to base on NVM version instead of FW version*/
        status = get_nvm_version(adapter, &nvm_version);
        if(status != DDP_SUCCESS)
        {
            *is_fw_supported = FALSE;
            break;
        }

        /* Compare firmware version with minimal supported firmware version */
        if(nvm_version.nvm_version_major > I40E_MIN_FW_VERSION_MAJOR)
        {
            *is_fw_supported = TRUE;
        }
        if(nvm_version.nvm_version_major == I40E_MIN_FW_VERSION_MAJOR &&
           nvm_version.nvm_version_minor >= I40E_MIN_FW_VERSION_MINOR)
        {
            *is_fw_supported = TRUE;
        }
    } while(0);

    return status;
}

bool
_i40e_is_virtual_function(adapter_t* adapter)
{
    if(adapter->device_id == LINUX_40G_VIRTUAL_DEVID ||
       adapter->device_id == BARLEYVILLE_40G_VIRTUAL_DEVID)
    {
        return TRUE;
    }

    return FALSE;
}

ddp_status_t
_i40e_get_ddp_profile_list(adapter_t* adapter)
{
    adminq_desc_t*     descriptor       = NULL;
    uint8_t*           data_buffer      = NULL;
    ddp_status_t       status           = DDP_SUCCESS;
    uint16_t           data_buffer_size = DDP_ADMINQ_WRITEBACK_SIZE;
    uint16_t           descriptor_size  = sizeof(adminq_desc_t) + DDP_ADMINQ_WRITEBACK_SIZE;

    do
    {
        descriptor  = malloc_sec(descriptor_size);
        if(descriptor == NULL)
        {
            status = DDP_ALLOCATE_MEMORY_FAIL;
            debug_ddp_print("Cannot allocate buffer\n");
            break;
        }

        memset(descriptor, 0, descriptor_size);

        descriptor->opcode   = I40E_ADMINQ_COMMAND_GET_DDP_PROFILE_LIST;
        descriptor->flags    = I40E_ADMINQ_FLAG_BUF;
        descriptor->datalen  = data_buffer_size;

        status = execute_adminq_command(adapter, descriptor, descriptor_size, data_buffer);
        if(status != DDP_SUCCESS)
        {
            status = DDP_AQ_COMMAND_FAIL;
            debug_ddp_print("execute_adminq_command status 0x%X\n", status);
            break;
        }
        else
        {
            status = memcpy_sec(&adapter->profile_info,
                                sizeof(profile_info_t),
                                &descriptor->params.raw[OFFSET_TO_ADMINQ_WRITEBACK],
                                sizeof(profile_info_t));
            if(adapter->profile_info.section_size == 0)
            {
                status = DDP_NO_DDP_PROFILE;
            }
        }
    } while(0);

    free_memory(descriptor);

    return status;
}

ddp_status_t
_i40e_discovery_device(adapter_t* adapter)
{
    ddp_status_t status          = DDP_SUCCESS;
    bool         is_fw_supported = FALSE;

    do
    {
        if(adapter->is_usable == FALSE)
            break; /* tool cannot read below data and need to copy them from other function */

        status = _i40e_check_fw_version(adapter, &is_fw_supported);
        if(status != DDP_SUCCESS)
        {
            debug_ddp_print("Error when checking FW version for currently printed adapter\n");
            break;
        }
        if(is_fw_supported == TRUE)
        {
            status = _i40e_get_ddp_profile_list(adapter);
            if(status == DDP_NO_DDP_PROFILE)
            {
                strcpy_sec(adapter->profile_info.name, DDP_PROFILE_NAME_LENGTH, "No profile loaded");
            }
        }
        else if(status == DDP_SUCCESS)
        {
            status = DDP_NO_SUPPORTED_ADAPTER;
            strcpy_sec(adapter->profile_info.name, DDP_PROFILE_NAME_LENGTH, "Unsupported FW version");
        }
    } while(0);

    if(status != DDP_SUCCESS               &&
       status != DDP_NO_DDP_PROFILE        &&
       status != DDP_NO_SUPPORTED_ADAPTER)
    {
        strcpy_sec(adapter->profile_info.name, DDP_PROFILE_NAME_LENGTH, "-");
    }

    return status;
}

ddp_status_t
i40e_verify_driver(void)
{
    char*                file_version_path   = "/sys/module/i40e/version";
    ddp_status_t         ddp_status          = DDP_SUCCESS;
    driver_os_version_t* i40e_driver_version = &Global_driver_os_ctx[family_40G].driver_version;

    do
    {
        /* check if driver is available */
        ddp_status = get_driver_version_from_os(i40e_driver_version, file_version_path);
        if(ddp_status != DDP_SUCCESS)
        {
            if(ddp_status == DDP_NO_BASE_DRIVER)
            {
                Global_driver_os_ctx[family_40G].driver_available = FALSE;
                Global_driver_os_ctx[family_40G].driver_supported = FALSE;
                break;
            }
            else if(ddp_status == DDP_UNSUPPORTED_BASE_DRIVER)
            {
                Global_driver_os_ctx[family_40G].driver_available = TRUE;
                Global_driver_os_ctx[family_40G].driver_supported = FALSE;
                break;
            }
        }
        else
        {
            Global_driver_os_ctx[family_40G].driver_available = TRUE;
        }

        /* check if driver is supported */
        if(i40e_driver_version->major < DDP_MIN_BASE_DRIVER_VERSION_MAJOR)
        {
            ddp_status = DDP_UNSUPPORTED_BASE_DRIVER;
            break;
        }

        if(i40e_driver_version->major == DDP_MIN_BASE_DRIVER_VERSION_MAJOR &&
           i40e_driver_version->minor < DDP_MIN_BASE_DRIVER_VERSION_MINOR)
        {
            ddp_status = DDP_UNSUPPORTED_BASE_DRIVER;
            break;
        }

        if(i40e_driver_version->major == DDP_MIN_BASE_DRIVER_VERSION_MAJOR &&
           i40e_driver_version->minor == DDP_MIN_BASE_DRIVER_VERSION_MINOR &&
           i40e_driver_version->build < DDP_MIN_BASE_DRIVER_VERSION_BUILD)
        {
            ddp_status = DDP_UNSUPPORTED_BASE_DRIVER;
            break;
        }
    } while(0);

    if(ddp_status == DDP_UNSUPPORTED_BASE_DRIVER)
    {
        Global_driver_os_ctx[family_40G].driver_supported = FALSE;
    }
    else
    {
        Global_driver_os_ctx[family_40G].driver_supported = TRUE;
    }

    if(Global_driver_os_ctx[family_40G].driver_supported == FALSE)
    {
        debug_ddp_print("Unsupported i40e driver version: %d.%d.%d\n",
                        i40e_driver_version->major,
                        i40e_driver_version->minor,
                        i40e_driver_version->build);
    }

    return ddp_status;
}

void
i40e_initialize_device(adapter_t* adapter)
{
    if(adapter != NULL)
    {
        adapter->tdi.discovery_device     = _i40e_discovery_device;
        adapter->tdi.is_virtual_function  = _i40e_is_virtual_function;
    }
}