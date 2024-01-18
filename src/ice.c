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

#include "ice.h"
#include "ddp.h"
#include "qdl_i.h"
#include "qdl_t.h"
#include "qdl_codes.h"
#include <time.h>

#define DDPT_IS_TYPE_ALIGNED(type, length)  ((sizeof(type) % (length)) == 0 ? true : false)
#define DDPT_TYPE_LENGTH(type, length)      (sizeof(type) / (length))

supported_devices_t ice_supported_devices[] =
{
        /*=============================================*/
        /* Vendor Device SubVen SubDev Branding String */
        /*---------------------------------------------*/
        /* E810 */
        {0x8086, 0x10A6, 0xFFFF, 0xFFFF, "Intel(R) E810-C Multi-Function Network Device"},
        {0x8086, 0x1590, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller E810-C"},
        {0x8086, 0x1591, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller E810-C for backplane"},
        {0x8086, 0x1592, 0x8086, 0x0001, "Intel(R) Ethernet Network Adapter E810-C-Q1"},
        {0x8086, 0x1592, 0x8086, 0x0002, "Intel(R) Ethernet Network Adapter E810-C-Q2"},
        {0x8086, 0x1592, 0x8086, 0x0005, "Intel(R) Ethernet Network Adapter E810-C-Q1 for OCP3.0"},
        {0x8086, 0x1592, 0x8086, 0x0006, "Intel(R) Ethernet Network Adapter E810-C-Q2 for OCP3.0"},
        {0x8086, 0x1592, 0x8086, 0x000D, "Intel(R) Ethernet Network Adapter E810-L-Q2 for OCP 3.0"},
        {0x8086, 0x1592, 0x8086, 0x0009, "Intel(R) Ethernet Network Adapter E810-C-Q1"},
        {0x8086, 0x1592, 0x8086, 0x000A, "Intel(R) Ethernet Network Adapter E810-C-Q1 for OCP"},
        {0x8086, 0x1592, 0x8086, 0x000E, "Intel(R) Ethernet Network Adapter E810-2C-Q2"},
        {0x8086, 0x1592, 0x1137, 0x02BF, "Cisco(R) E810CQDA2 2x100 GbE QSFP28 PCIe NIC"},
        {0x8086, 0x1593, 0x1137, 0x02C3, "Cisco(R) E810XXVDA4 4x25/10 GbE SFP28 PCIe NIC"},
        {0x8086, 0x159B, 0x1137, 0x02BE, "Cisco(R) E810XXVDA2 2x25/10 GbE SFP28 PCIe NIC"},
        {0x8086, 0x1593, 0x1028, 0x09D4, "Intel(R) Ethernet Controller E810-C for SFP"},
        {0x8086, 0x1593, 0x1028, 0x09D5, "Intel(R) Ethernet Controller E810-C for SFP"},
        {0x8086, 0x1592, 0x1590, 0x0324, "HPE Synergy 7610C 50/100Gb Ethernet Adapter"},
        {0x8086, 0x1593, 0x8086, 0x0001, "Intel(R) Ethernet Network Adapter E810-L-1"},
        {0x8086, 0x1593, 0x8086, 0x0005, "Intel(R) Ethernet Network Adapter E810-XXV-4"},
        {0x8086, 0x1593, 0x8086, 0x0007, "Intel(R) Ethernet Network Adapter E810-XXV-4"},
        {0x8086, 0x1593, 0x8086, 0x000C, "Intel(R) Ethernet Network Adapter E810-XXV-4 for OCP 3.0"},
        {0x8086, 0x1593, 0x8086, 0x000D, "Intel(R) Ethernet 25G 4P E810-XXV OCP"},
        {0x8086, 0x1593, 0x8086, 0x000E, "Intel(R) Ethernet Network Adapter E810-XXV-4T"},
        {0x8086, 0x1592, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller E810-C for QSFP"},
        {0x8086, 0x1593, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller E810-C for SFP"},
        {0x8086, 0x1598, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller E810-XXV"},
        {0x8086, 0x1599, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller E810-XXV for backplane"},
        {0x8086, 0x159A, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller E810-XXV for QSFP"},
        {0x8086, 0x159B, 0x8086, 0x0001, "Intel(R) Ethernet 25G 2P E810-XXV OCP"},
        {0x8086, 0x159B, 0x8086, 0x0002, "Intel(R) Ethernet 25G 2P E810-XXV Adapter"},
        {0x8086, 0x159B, 0x8086, 0x0003, "Intel(R) Ethernet Network Adapter E810-XXV-2"},
        {0x8086, 0x159B, 0x8086, 0x0004, "Intel(R) Ethernet Network Adapter E810-XXV-2"},
        {0x8086, 0x159B, 0x8086, 0x0005, "Intel(R) Ethernet Network Adapter E810-XXV-2 for OCP 3.0"},
        {0x8086, 0x159B, 0x8086, 0x0006, "Intel(R) Ethernet Network Adapter E810-XXV-2 for OCP 3.0"},
        {0x8086, 0x159B, 0x8086, 0x4001, "Intel(R) Ethernet Network Adapter E810-XXV-2"},
        {0x8086, 0x159B, 0x8086, 0x4002, "Intel(R) Ethernet Network Adapter E810-XXV-2 for OCP 3.0"},
        {0x8086, 0x159B, 0x8086, 0x4003, "Intel(R) Ethernet Network Adapter E810-XXV-2"},
        {0x8086, 0x159B, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller E810-XXV for SFP"},
        {0x8086, 0x159C, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller E810-XXV/X557-AT 10GBASE-T"},
        {0x8086, 0x159D, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Controller E810-XXV 1GbE"},
        /* Adaptive VF */
        {0x8086, 0x1889, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Adaptive Virtual Function"},
        /* E822 */
        {0x8086, 0x124C, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Connection E823-L for backplane"},
        {0x8086, 0x124D, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Connection E823-L for SFP"},
        {0x8086, 0x124E, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Connection E823-L/X557-AT 10GBASE-T"},
        {0x8086, 0x124F, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Connection E823-L 1GbE"},
        {0x8086, 0x151D, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Connection E823-L for QSFP"},
        {0x8086, 0x1890, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Connection E822-C for backplane"},
        {0x8086, 0x1891, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Connection E822-C for QSFP"},
        {0x8086, 0x1892, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Connection E822-C for SFP"},
        {0x8086, 0x1893, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Connection E822-C/X557-AT 10GBASE-T"},
        {0x8086, 0x1894, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Connection E822-C 1GbE"},
        {0x8086, 0x1897, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Connection E822-L for backplane"},
        {0x8086, 0x1898, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Connection E822-L for SFP"},
        {0x8086, 0x1899, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Connection E822-L/X557-AT 10GBASE-T"},
        {0x8086, 0x189A, 0xFFFF, 0xFFFF, "Intel(R) Ethernet Connection E822-L 1GbE"}
};

uint16_t ice_supported_devices_size = sizeof(ice_supported_devices)/sizeof(ice_supported_devices[0]);

/* Function checks if provided adapter is associated with virtual function.
 *
 * Parameters:
 * [in,out] adapter      Handle to adapter
 *
 * Returns: DDP status.
 */
bool
_ice_is_virtual_function(adapter_t* adapter)
{
    if(adapter != NULL && adapter->device_id == ICE_DEV_ID_ADAPTIVE_VF)
    {
        return TRUE;
    }

    return FALSE;
}

/* Function acquires adminq by writing lock value to AQ related registers.
 *
 * Parameters:
 * [in,out] adapter      Handle to adapter
 *
 * Returns: DDP status.
 */
ddp_status_t
_ice_acquire_adminq(adapter_t* adapter)
{
    ddp_status_t status                = DDP_SUCCESS;
    time_t       current_timestamp     = 0;
    time_t       last_timestamp        = 0;
    int64_t      timestamp_delta       = 0;
    uint32_t     hicr_en               = 0;
    uint32_t     hicr                  = 0;
    uint32_t     hida                  = 0;
    uint32_t     input_register_buffer = ICE_LOCK_SEMAPHORE_VALUE;

    do
    {
        /* Check input argument */
        if(adapter == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        /* check if CSR mechanism is enabled */
        status = read_register(adapter, ICE_GL_HICR_EN_REGISTER, DDP_DWORD_LENGTH, &hicr_en);
        if(status != DDP_SUCCESS)
        {
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            break;
        }

        if((hicr_en & ICE_GL_HICR_EN_ENABLE_BIT) == 0)
        {
            debug_ddp_print("Access to device by CSR is not enabled\n");
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            break;
        }

        status = read_register(adapter, ICE_GL_HICR_REGISTER, DDP_DWORD_LENGTH, &hicr);
        if(status != DDP_SUCCESS)
        {
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            break;
        }

        if((hicr & ICE_GL_HICR_COMMAND_BIT) != 0)
        {
            debug_ddp_print("Resources not available\n");
            status = DDP_ADAPTER_ERROR;
            break;
        }

        /* check if CSR mechanism is busy */
        status = read_register(adapter, GL_HIDA(0), DDP_DWORD_LENGTH, &hida);
        if(status != DDP_SUCCESS)
        {
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            break;
        }

        current_timestamp = time(NULL);

        if(upper_16_bits(hida) != 0xFFFF && upper_16_bits(hida) != 0)
        {
            /* CSR mechanism is marked as busy, but may have expired due to long inactivity time.
             * Check if it can be acquired. */
            status = read_register(adapter,
                                   GL_HIDA(ICE_DESC_COOKIE_L_DWORD_OFFSET),
                                   DDP_DWORD_LENGTH,
                                   &last_timestamp);
            if(status != DDP_SUCCESS)
            {
                status = DDP_CANNOT_COMMUNICATE_ADAPTER;
                break;
            }

            timestamp_delta = current_timestamp - last_timestamp;
            if(last_timestamp == 0 ||
               (timestamp_delta >= ICE_TOOLSQ_EXPIRED_STAMP_SPACING_LO &&
                timestamp_delta <= ICE_TOOLSQ_EXPIRED_STAMP_SPACING_HI))
            {
                debug_ddp_print("AQ lock failed, it was locked %ld seconds ago.\n", timestamp_delta);
                status = DDP_CANNOT_COMMUNICATE_ADAPTER;
                break;
            }

            debug_ddp_print("AQ was locked %ld seconds ago, lock is considered timed-out.\n", timestamp_delta);
        }

        /* lock adminQ by CSR */
        status = write_register(adapter, GL_HIDA(0), DDP_DWORD_LENGTH, &input_register_buffer);
        if(status != DDP_SUCCESS)
        {
            debug_ddp_print("%d - write_register status: 0x%X\n", __LINE__, status);
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            break;
        }

        status = write_register(adapter,
                                GL_HIDA(ICE_DESC_COOKIE_L_DWORD_OFFSET),
                                DDP_DWORD_LENGTH,
                                &current_timestamp);
        if(status != DDP_SUCCESS)
        {
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            break;
        }
    } while (0);

    return status;
}

/* Function releases adminq by clearing value proper AQ related registers.
 * 
 * Parameters:
 * [in,out] adapter      Handle to adapter
 *
 * Returns: DDP status.
 */
ddp_status_t
_ice_release_adminq(adapter_t* adapter)
{
    ddp_status_t status                = DDP_SUCCESS;
    uint32_t     input_register_buffer = 0xFFFFFFFF;

    do
    {
        if(adapter == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }
        status = write_register(adapter, GL_HIDA(0), DDP_DWORD_LENGTH, &input_register_buffer);
        if(status != DDP_SUCCESS)
        {
            debug_ddp_print("release AdminQ failed!\n");
            debug_ddp_print("write_register status: 0x%X\n", status);
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
        }
    } while(0);

    return status;
}

/* Function sends adminq command.
 *
 * Parameters:
 * [in,out] adapter         Handle to adapter
 * [in]     descriptor      Admin Queue descriptor
 *
 * Returns: DDP status.
 */
ddp_status_t
_ice_send_adminq_command(adapter_t* adapter, adminq_desc_t* descriptor)
{
    uint32_t*    desc   = (uint32_t*)descriptor;
    ddp_status_t status = DDP_SUCCESS;
    uint32_t     i      = 0;
    uint32_t     hicr   = 0;

    do
    {
        /* Check input arguments */
        if(adapter == NULL || descriptor == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        for(i = 0; i < sizeof(adminq_desc_t) / DDP_DWORD_LENGTH; i++)
        {
            status = write_register(adapter, GL_HIDA(i), DDP_DWORD_LENGTH, &desc[i]);
            if(status != DDP_SUCCESS)
            {
                debug_ddp_print("%d: send adminQ failed during writing register [iteration %d]\n", __LINE__, i);
                debug_ddp_print("write register failed: 0x%X\n", status);
                break;
            }
        }
        if(status != DDP_SUCCESS)
        {
            break;
        }

        /* send information to the fw about finished writing adminq command */
        status = read_register(adapter, ICE_GL_HICR_REGISTER, DDP_DWORD_LENGTH, &hicr);
        if(status != DDP_SUCCESS)
        {
            debug_ddp_print("%d: read register failed: 0x%X\n", __LINE__, status);
            break;
        }
        hicr = (hicr | ICE_GL_HICR_COMMAND_BIT) & ~(ICE_GL_HICR_STATUS_VALID_BIT | ICE_GL_HICR_EVENT_VALID_BIT);
        status = write_register(adapter, ICE_GL_HICR_REGISTER, DDP_DWORD_LENGTH, &hicr);
        if(status != DDP_SUCCESS)
        {
            debug_ddp_print("%d: write register failed: 0x%X\n", __LINE__, status);
            break;
        }
    } while (0);

    return status;
}

ddp_status_t
_ice_recv_adminq_command(adapter_t* adapter, adminq_desc_t* descriptor)
{
    adminq_desc_t received_descriptor;
    uint32_t*     desc        = (uint32_t*)&received_descriptor;
    ddp_status_t  status      = DDP_SUCCESS;
    uint32_t      i           = 0;
    uint32_t      desc_length = DDPT_TYPE_LENGTH(adminq_desc_t, DDP_DWORD_LENGTH);

    /* Check asserts */
    _Static_assert(DDPT_IS_TYPE_ALIGNED(adminq_desc_t, DDP_DWORD_LENGTH), "adminq_desc_t type not aligned.");

    MEMINIT(&received_descriptor);

    do
    {
        /* Check inputs */
        if(adapter == NULL || descriptor == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        for(i = 0; i < desc_length; i++)
        {
            status = read_register(adapter, GL_HIDA(i), DDP_DWORD_LENGTH, &desc[i]);
            if(status != DDP_SUCCESS)
            {
                debug_ddp_print("%d: send adminQ failed during read register 0x%X\n", __LINE__, GL_HIDA(i));
                debug_ddp_print("read register failed: 0x%X\n", status);
                break;
            }
        }

        /* validate response */
        if(status != DDP_SUCCESS                                            ||
           received_descriptor.opcode != descriptor->opcode                 ||
           received_descriptor.retval != AQ_EOK                             ||
           (received_descriptor.flags & ICE_AQ_FLAG_ERR) == ICE_AQ_FLAG_ERR)
        {
            status = DDP_AQ_COMMAND_FAIL;
            debug_ddp_print("AdminQ command failed! Returned error code: %u\n", received_descriptor.retval);
        }
        else
        {
            memcpy_sec(descriptor, sizeof(adminq_desc_t), desc, sizeof(received_descriptor));
        }
    } while(0);

    return status;
}

ddp_status_t
_ice_recv_adminq_buffer(adapter_t* adapter, uint8_t* buffer, uint32_t buffer_size)
{
    ddp_status_t status = DDP_SUCCESS;
    uint32_t     i      = 0;

    for(i = 0; i < buffer_size / DDP_DWORD_LENGTH ; i++)
    {
        status = read_register(adapter, GL_HIBA(i), DDP_DWORD_LENGTH, &buffer[i * DDP_DWORD_LENGTH]);
        if(status != DDP_SUCCESS)
        {
            debug_ddp_print("%d: send adminQ failed during read register 0x%X\n", __LINE__, GL_HIBA(i));
            debug_ddp_print("read register failed: 0x%X\n", status);
            break;
        }
    }

    return status;
}

ddp_status_t
_ice_get_data_by_csr(adapter_t* adapter, adminq_desc_t* descriptor, uint8_t* buffer, uint16_t buffer_size)
{
    ddp_status_t status             = DDP_SUCCESS;
    uint32_t     hicr               = 0;
    uint32_t     i                  = 0;
    bool         is_adminq_acquired = FALSE;

    do
    {
        /* aquire AdminQ */
        status = _ice_acquire_adminq(adapter);
        if(status != DDP_SUCCESS)
        {
            debug_ddp_print("_ice_acquire_adminq 0x%X\n", status);
            _ice_release_adminq(adapter);
            break;
        }

        is_adminq_acquired = TRUE;

        /* send request to the FW */
        status = _ice_send_adminq_command(adapter, descriptor);
        if(status != DDP_SUCCESS)
        {
            debug_ddp_print("_ice_send_adminq_command error 0x%X\n", status);
            break;
        }

        /* timeout */
        for(i = 0; i < 100000; i++)
        {
            status = read_register(adapter, ICE_GL_HICR_REGISTER, DDP_DWORD_LENGTH, &hicr);
            if(status != DDP_SUCCESS)
            {
                break;
            }
            if((hicr & ICE_GL_HICR_STATUS_VALID_BIT) != 0 ||
               (hicr & ICE_GL_HICR_COMMAND_BIT)      == 0)
            {
                break;
            }
            usleep(1);
        }

        /* received data form host interface descriptor area (hida) */
        status = _ice_recv_adminq_command(adapter, descriptor);
        if(status != DDP_SUCCESS)
        {
            debug_ddp_print("_ice_recv_adminq_command error 0x%X\n", status);
            break;
        }

        /* the respons is expected as a part of descriptor */
        if(buffer == NULL)
        {
            break;
        }

        /* received data form host interface buffer area (hiba) */
        status = _ice_recv_adminq_buffer(adapter, buffer, buffer_size);
        if(status != DDP_SUCCESS)
        {
            debug_ddp_print("_ice_recv_adminq_buffer error 0x%X\n", status);
            break;
        }
    } while (0);

    if(is_adminq_acquired == TRUE)
    {
        /* release AdminQ */
        status = _ice_release_adminq(adapter);
        if(status != DDP_SUCCESS)
        {
            debug_ddp_print("_ice_release_adminq error 0x%X\n", status);
        }
    }

    return status;
}

ddp_status_t
_ice_check_fw_version(adapter_t* adapter, bool* is_fw_supported, ddp_descriptor_t* dscr)
{
    ddp_status_t       status             = DDP_SUCCESS;
    adminq_desc_t*     descriptor         = (adminq_desc_t*)dscr->descriptor;
    ice_aqc_get_ver_t* get_version        = NULL;

    do
    {
        descriptor->opcode  = ICE_ADMINQ_COMMAND_GET_VERSION;
        descriptor->flags   = ICE_AQ_FLAG_SI;
        descriptor->datalen = 0;

        status = _ice_get_data_by_csr(adapter, descriptor, NULL, 0);
        if(status != DDP_SUCCESS)
        {
            debug_ddp_print("ice_csr_get_profile_info error: 0x%X\n", status);
            break;
        }

        get_version = (ice_aqc_get_ver_t*)descriptor->params.raw;

        debug_ddp_print("fw version: 0x%X 0x%X 0x%X 0x%X 0x%X\n",
                        get_version->fw_branch,
                        get_version->fw_major,
                        get_version->fw_minor,
                        get_version->fw_patch,
                        get_version->fw_build);

        if(get_version->fw_major > ICE_MIN_FW_VERSION_MAJOR)
        {
            *is_fw_supported = TRUE;
        }
        else if(get_version->fw_major == ICE_MIN_FW_VERSION_MAJOR  &&
                get_version->fw_minor >  ICE_MIN_FW_VERSION_MINOR)
        {
            *is_fw_supported = TRUE;
        }
        else if(get_version->fw_major == ICE_MIN_FW_VERSION_MAJOR  &&
                get_version->fw_minor == ICE_MIN_FW_VERSION_MINOR  &&
                get_version->fw_patch >= ICE_MIN_FW_VERSION_PATCH)
        {
            *is_fw_supported = TRUE;
        }
    } while(0);

    return status;
}

ddp_status_t
_ice_get_adminq_ddp_profile_list(adapter_t* adapter, ddp_descriptor_t* dscr)
{
    ice_profiles_info_t  profiles_info;
    ddp_status_t         status         = DDP_SUCCESS;
    adminq_desc_t*       descriptor     = (adminq_desc_t*)dscr->descriptor;
    uint8_t              profile_number = 0;

    MEMINIT(&profiles_info);

    debug_ddp_print("Trying to read profile by AdminQ inteface.\n");

    do
    {
        descriptor->opcode  = ICE_ADMINQ_COMMAND_GET_DDP_PROFILE_LIST;
        descriptor->flags   = ICE_AQ_FLAG_BUF | ICE_AQ_FLAG_SI;
        descriptor->datalen = sizeof profiles_info;

        status = _ice_get_data_by_csr(adapter, descriptor, (uint8_t*)&profiles_info, sizeof profiles_info);
        if(status != DDP_SUCCESS)
        {
            debug_ddp_print("ice_get_data_by_basedriver 0x%X", status);
            break;
        }

        /* Copy data from ICE strcute to the generic tool structure */
        if(profiles_info.count == 0)
        {
            status = DDP_NO_DDP_PROFILE;
            break;
        }

        debug_ddp_print("Found %d profile(s)\n", profiles_info.count);

        adapter->profile_info.section_size = profiles_info.count;
        for(profile_number = 0; profile_number < ICE_PROFILES_NUMBER; profile_number++)
        {
            debug_ddp_print("Profile: %d:\n", profile_number);
            debug_ddp_print("  IsActive: %d\n", profiles_info.profile[profile_number].is_active);
            debug_ddp_print("  Name:     %s\n", profiles_info.profile[profile_number].name);
            debug_ddp_print("  TrackId:  %x\n", profiles_info.profile[profile_number].track_id);
            /* The tool shall reports only active profile */
            if(profiles_info.profile[profile_number].is_active == TRUE)
            {
                adapter->profile_info.version      = profiles_info.profile[profile_number].version;
                adapter->profile_info.track_id     = profiles_info.profile[profile_number].track_id;
                memcpy_sec(adapter->profile_info.name,
                           DDP_PROFILE_NAME_LENGTH,
                           profiles_info.profile[profile_number].name,
                           ICE_PROFILE_NAME_LENGTH);
                break;
            }
        }
    } while (0);

    return status;
}

ddp_status_t
_ice_get_devlink_profile_info(adapter_t* adapter, ddp_descriptor_t* dscr)
{
    char                   profile_version[DDP_VERSION_LENGTH];
    char                   track_id[DDP_TRACKID_LENGTH];
    qdl_dscr_t             qdl_descriptor = (qdl_dscr_t)dscr->descriptor;
    ddp_profile_version_t* version        = &adapter->profile_info.version;
    uint8_t*               msg            = 0;
    uint8_t*               rec_msg        = 0;
    ddp_status_t           status         = DDP_SUCCESS;
    qdl_status_t           qdl_status     = QDL_SUCCESS;
    uint32_t               msg_size       = 0;
    uint32_t               rec_msg_size   = QDL_REC_BUFF_SIZE;

    memset(profile_version, '\0', DDP_VERSION_LENGTH);
    memset(track_id, '\0', DDP_TRACKID_LENGTH);

    debug_ddp_print("Trying to read profile by DevLink inteface.\n");

    do
    {
        if(adapter == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        msg = qdl_create_msg(qdl_descriptor, QDL_CMD_INFO_GET, &msg_size, NULL);
        if(msg == NULL)
        {
            debug_ddp_print("qdl_create_msg error\n");
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            break;
        }

        qdl_status = qdl_send_msg(qdl_descriptor, msg, msg_size);
        if(qdl_status != QDL_SUCCESS)
        {
            debug_ddp_print("qdl_send_msg error 0x%X\n", qdl_status);
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            break;
        }

        rec_msg = malloc_sec(rec_msg_size);
        if(rec_msg == NULL)
        {
            debug_ddp_print("memory allocate error\n");
            status = DDP_ALLOCATE_MEMORY_FAIL;
            break;
        }

        qdl_status = qdl_receive_msg(qdl_descriptor, rec_msg, &rec_msg_size);
        if(qdl_status != QDL_SUCCESS)
        {
            debug_ddp_print("qdl_receive_msg error 0x%X\n", qdl_status);
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            break;
        }

        qdl_get_string_by_key(qdl_descriptor,
                              rec_msg,
                              rec_msg_size,
                              "fw.app.name",
                              adapter->profile_info.name,
                              DDP_PROFILE_NAME_LENGTH);

        qdl_get_string_by_key(qdl_descriptor,
                              rec_msg,
                              rec_msg_size,
                              "fw.app",
                              profile_version,
                              DDP_VERSION_LENGTH);
        sscanf(profile_version,
               "%hhu.%hhu.%hhu.%hhu",
               &version->major,
               &version->minor,
               &version->update,
               &version->draft);

        qdl_get_string_by_key(qdl_descriptor, rec_msg, rec_msg_size, "fw.app.bundle_id", track_id, DDP_TRACKID_LENGTH);

        sscanf(track_id, "%X", &adapter->profile_info.track_id);
        /* The AdminQ respons include the section size - this value is usefull to correct display the table
         * For DevLink this sections doesn't exists, let's set the dummy value (greater than 0).
         */
        adapter->profile_info.section_size = 1;
    } while (0);

    free_memory(msg);
    free_memory(rec_msg);

    return status;
}

void
_ice_get_adapter_descriptor(adapter_t* adapter, ddp_descriptor_t* dscr)
{
    qdl_dscr_t     qdl_descriptor = NULL;
    adminq_desc_t* aq_descriptor  = NULL;

    dscr->descriptor_type = descriptor_none;

    do
    {
        if(adapter->is_virtual_function == TRUE)
        {
            /* For virtual function the tool shall use the PF location to read data */
            qdl_descriptor = qdl_init_dev(adapter->pf_location.segment,
                                          adapter->pf_location.bus,
                                          adapter->pf_location.device,
                                          adapter->pf_location.function,
                                          QDL_INIT_NVM);
        }
        else
        {
            qdl_descriptor = qdl_init_dev(adapter->location.segment,
                                          adapter->location.bus,
                                          adapter->location.device,
                                          adapter->location.function,
                                          QDL_INIT_NVM);
        }

        if(qdl_descriptor != NULL)
        {
            debug_ddp_print("Tool will use the DevLink interface for this device\n");
            dscr->descriptor = qdl_descriptor;
            dscr->descriptor_type = descriptor_devlink;
            break;
        }

        /* Check if name for Ethernet interface is available */
        if(adapter->is_usable == FALSE)
            break;

        aq_descriptor = malloc_sec(sizeof(adminq_desc_t));
        if(aq_descriptor == NULL)
        {
            debug_ddp_print("Cannot allocate buffer\n");
            break;
        }

        debug_ddp_print("Tool will use the IOCTL interface for this device\n");
        dscr->descriptor = aq_descriptor;
        dscr->descriptor_type = descriptor_ioctl;
    } while (0);
}

void
ice_release_descriptor(ddp_descriptor_t* dscr)
{
    if(dscr->descriptor_type == descriptor_devlink)
    {
        qdl_release_dev((qdl_dscr_t)dscr->descriptor);
    }
    else if(dscr->descriptor_type == descriptor_ioctl)
    {
        free_memory(dscr->descriptor);
    }
}

ddp_status_t
_ice_discovery_device(adapter_t* adapter)
{
    ddp_descriptor_t descriptor;
    ddp_status_t     status          = DDP_SUCCESS;
    bool             is_fw_supported = FALSE;

    MEMINIT(&descriptor);

    do
    {
        _ice_get_adapter_descriptor(adapter, &descriptor);
        if(descriptor.descriptor == NULL)
        {
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            break;
        }

        if(descriptor.descriptor_type == descriptor_ioctl)
        {
            status = _ice_check_fw_version(adapter, &is_fw_supported, &descriptor);
            if(status != DDP_SUCCESS)
            {
                debug_ddp_print("Error when checking FW version for currently printed adapter\n");
                break;
            }

            if(is_fw_supported == FALSE)
            {
                status = DDP_NO_SUPPORTED_ADAPTER;
                strcpy_sec(adapter->profile_info.name,
                           DDP_PROFILE_NAME_LENGTH,
                           UNSUPPORTED_FW,
                           strlen(UNSUPPORTED_FW));
                break;
            }
            status = _ice_get_adminq_ddp_profile_list(adapter, &descriptor);
            if(status == DDP_NO_DDP_PROFILE)
            {
                strcpy_sec(adapter->profile_info.name,
                           DDP_PROFILE_NAME_LENGTH,
                           NO_PROFILE,
                           strlen(NO_PROFILE));
            }
        }
        else if(descriptor.descriptor_type == descriptor_devlink)
        {
            status = _ice_get_devlink_profile_info(adapter, &descriptor);
        }
        else
        {
            debug_ddp_print("Cannot create interface descriptor\n");
        }
    } while(0);

    if(status != DDP_SUCCESS               &&
       status != DDP_NO_DDP_PROFILE        &&
       status != DDP_NO_SUPPORTED_ADAPTER)
    {
        strcpy_sec(adapter->profile_info.name,
                   DDP_PROFILE_NAME_LENGTH,
                   EMPTY_MESSAGE,
                   strlen(EMPTY_MESSAGE));
    }

    ice_release_descriptor(&descriptor);

    return status;
}

ddp_status_t
_100g_verify_driver(char* driver_name, driver_os_version_t* ice_driver_version)
{
    char                 ice_driver_path[DDP_MAX_BUFFER_SIZE];
    char                 ice_module_version_path[DDP_MAX_BUFFER_SIZE];
    DIR*                 dir                     = NULL;
    ddp_status_t         ddp_status              = DDP_SUCCESS;

    memset(ice_driver_path, '\0', sizeof(ice_driver_path));
    memset(ice_module_version_path, '\0', sizeof(ice_module_version_path));

    do
    {
        if(driver_name == NULL)
        {
            ddp_status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        snprintf(ice_driver_path, DDP_MAX_BUFFER_SIZE, "/sys/bus/pci/drivers/%s", driver_name);
        snprintf(ice_module_version_path, DDP_MAX_BUFFER_SIZE, "/sys/module/%s/version", driver_name);

        debug_ddp_print("driver patch: %s\n", ice_driver_path);
        debug_ddp_print("module version path: %s\n", ice_module_version_path);
        
        /* check if driver is available */
        ddp_status = get_driver_version_from_os(ice_driver_version, ice_module_version_path);
        if(ddp_status == DDP_SUCCESS)
        {
            debug_ddp_print("%s driver version: %d.%d.%d\n",
                            driver_name,
                            ice_driver_version->major,
                            ice_driver_version->minor,
                            ice_driver_version->build);
        }
        else
        {
            /* All ICE drivers are supported, so if the driver version is not available just check if driver exists, so
            * let's check if the directory /sys/bus/pci/drivers/ice exists.
            */
            dir = opendir(ice_driver_path);
            if(dir != NULL)
            {
                closedir(dir);
                debug_ddp_print("%s driver version not available\n", driver_name);
                /* the driver extis in the system, the success shall be returned. */
                ddp_status = DDP_SUCCESS;
            }
            else
            {
                debug_ddp_print("%s driver not available\n", driver_name);
            }
        }
    } while(0);

    return ddp_status;
}

ddp_status_t
ice_verify_driver(void)
{
    ddp_status_t         status             = DDP_SUCCESS;
    driver_os_version_t* ice_driver_version = &Global_driver_os_ctx[family_100G].driver_version;

    status = _100g_verify_driver("ice", ice_driver_version);
    if(status == DDP_SUCCESS)
    {
        Global_driver_os_ctx[family_100G].driver_available = TRUE;
        Global_driver_os_ctx[family_100G].driver_supported = TRUE;
    }

    return status;
}

ddp_status_t
ice_sw_verify_driver(void)
{
    ddp_status_t         status             = DDP_SUCCESS;
    driver_os_version_t* ice_driver_version = &Global_driver_os_ctx[family_100G_SW].driver_version;

    status = _100g_verify_driver("ice_sw", ice_driver_version);
    if(status == DDP_SUCCESS)
    {
        Global_driver_os_ctx[family_100G_SW].driver_available = TRUE;
        Global_driver_os_ctx[family_100G_SW].driver_supported = TRUE;
    }

    return status;
}

ddp_status_t
ice_swx_verify_driver(void)
{
    ddp_status_t         status             = DDP_SUCCESS;
    driver_os_version_t* ice_driver_version = &Global_driver_os_ctx[family_100G_SWX].driver_version;

    status = _100g_verify_driver("ice_swx", ice_driver_version);
    if(status == DDP_SUCCESS)
    {
        Global_driver_os_ctx[family_100G_SWX].driver_available = TRUE;
        Global_driver_os_ctx[family_100G_SWX].driver_supported = TRUE;
    }

    return status;
}

void
ice_initialize_device(adapter_t* adapter)
{
    if(adapter != NULL)
    {
        adapter->tdi.discovery_device     = _ice_discovery_device;
        adapter->tdi.is_virtual_function  = _ice_is_virtual_function;
    }
}
