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

#include "ddp.h"
#include "cmdparams.h"

driver_os_context_t Global_driver_os_ctx[family_last];

extern supported_devices_t i40e_supported_devices[];
extern uint16_t            i40e_supported_devices_size;
extern supported_devices_t ice_supported_devices[];
extern uint16_t            ice_supported_devices_size;
extern uint32_t            unsupported_i40e_device_ids[];
extern uint32_t            unsupported_i40e_array_size;

ddp_output_function_t ddp_func_print_adapter_list = generate_table;

ddp_status_t
validate_output_status(ddp_status_t status)
{
    /* For status equal or higher than 100, set generic return status */
    if(status >= DDP_AQ_COMMAND_FAIL)
    {
        /* TODO: Add debug log to print status value before overwriting */
        status = DDP_INTERNAL_GENERIC_ERROR;
    }

    return status;
}

char*
get_error_message(ddp_status_value_t status)
{
    char* message = "";

    switch(status)
    {
    case DDP_SUCCESS:
        message = "Success";
        break;
    case DDP_BAD_COMMAND_LINE_PARAMETER:
        message = "Bad command line parameter";
        break;
    case DDP_INTERNAL_GENERIC_ERROR:
        message = "An internal error has occurred";
        break;
    case DDP_INSUFFICIENT_PRIVILEGES:
        message = "Insufficient privileges to run the tool";
        break;
    case DDP_NO_SUPPORTED_ADAPTER:
        message = "No supported adapter found";
        break;
    case DDP_NO_BASE_DRIVER:
        message = "No driver available";
        break;
    case DDP_UNSUPPORTED_BASE_DRIVER:
        message = "Unsupported base driver version";
        break;
    case DDP_CANNOT_COMMUNICATE_ADAPTER:
        message = "Cannot communicate with one or more adapters";
        break;
    case DDP_NO_DDP_PROFILE:
        message = "Lack of DDP profiles on all devices";
        break;
    case DDP_CANNOT_READ_DEVICE_DATA:
        message = "Cannot read all information from one or more devices";
        break;
    case DDP_CANNOT_CREATE_OUTPUT_FILE:
        message = "Cannot create output file";
        break;
    case DDP_DEVICE_NOT_FOUND:
        message = "Cannot find specific devices" ;
        break;
    default:
        message = "";
        break;
    }
    return message;
}

void
free_memory(void* pointer)
{
    if(pointer != NULL)
    {
        free(pointer);
    }
}

bool
is_virtual_function(adapter_t* adapter)
{
    bool is_virtual = FALSE;

    if(adapter->tdi.is_virtual_function != NULL)
    {
        is_virtual = adapter->tdi.is_virtual_function(adapter);
    }

    return is_virtual;
}

/* Function get_brading_string_from_table() find the branding string for a 
 * given device in internal table.
 *
 * The recognize is done by matching device's 4-partID (VendorID, DeviceID,
 * SubvendorID and SubdeviceID) and if that fails - 2-partID (VendorID and
 * DeviceID). If a match is found this function sets adapter->branding_string
 * value for the current adapter. The list of legal devices is stored in
 * supported devices lists specific for each family. This matching assumes that
 * supported devices lists use 0xFFFF values in 4-partID entries as generic values.
 *
 * Parameters:
 * [in,out] adapter      Handle to current adapter
 * [out]    match_level  4 if 4-PartId match, 2 if 2-PartId match, 0 if no match
 *
 * Returns: None
 */
void
get_brading_string_from_table(adapter_t* adapter, match_level* match_level)
{
    supported_devices_t* supported_devices      = NULL;
    adapter_family_t     adapter_family         = family_none;
    uint16_t             supported_devices_size = 0;
    uint16_t             i                      = 0;
    uint16_t             a_ven_id               = adapter->vendor_id;
    uint16_t             a_dev_id               = adapter->device_id;
    uint16_t             a_sub_ven_id           = adapter->subvendor_id;
    uint16_t             a_sub_dev_id           = adapter->subdevice_id;
    uint16_t             generic_id             = 0xFFFF;
    bool                 is_supported           = FALSE;

    for(++adapter_family; adapter_family < family_last; adapter_family++)
    {
        switch(adapter_family)
        {
        case family_40G:
            supported_devices = i40e_supported_devices;
            supported_devices_size = i40e_supported_devices_size;
            break;
        case family_100G:
            supported_devices = ice_supported_devices;
            supported_devices_size = ice_supported_devices_size;
            break;
        case family_none:
            /* fall-through */
        case family_last:
            /* fall-through */
        default:
            supported_devices = NULL;
            supported_devices_size = 0;
            break;
        }

        do
        {
            for(i = 0; i < supported_devices_size; i++)
            {
                if(a_ven_id     == supported_devices[i].vendorid    &&
                   a_dev_id     == supported_devices[i].deviceid    &&
                   a_sub_ven_id == supported_devices[i].subvendorid &&
                   a_sub_dev_id == supported_devices[i].subdeviceid
                  )
                {
                    if(adapter->branding_string_allocated == TRUE)
                    {
                        free_memory(adapter->branding_string);
                        adapter->branding_string_allocated = FALSE;
                    }
                    adapter->branding_string = supported_devices[i].branding_string;
                    is_supported = TRUE;
                    adapter->adapter_family = adapter_family;
                    *match_level = four_part_id_match;
                    break;
                }
            }
            /* if tool doesn't find match for generic sub_ven & sub_dev the loop shall be broken */
            if(((a_sub_ven_id & a_sub_dev_id) == generic_id) && (is_supported == FALSE))
            {
                break;
            }
            if(is_supported == TRUE)
            {
                if((a_sub_dev_id & a_sub_ven_id) == generic_id) /* 2-partID match */
                {
                    *match_level = device_id_match;
                }
                else /* 4-partID match */
                {
                    *match_level = four_part_id_match;
                }

                break;
            }
            else
            {
                /* 4-partID matching failed - try 2-partID */
                a_sub_ven_id = generic_id;
                a_sub_dev_id = generic_id;
            }
        } while(TRUE);

        if(is_supported == TRUE)
        {
            /* match found, exit search */
            break;
        }
        /* reset sub fields for the next family */
        a_sub_ven_id = adapter->subvendor_id;
        a_sub_dev_id = adapter->subdevice_id;
    }
}

/* Function verifies if there is a supported driver attached to that specific device.
 * ice - all devices support ddp profiles.
 * i40e - Fortville with appropriate FW support ddp profiles (FW check required).
 * i40e - non-supported i40e devices are filtered using a device id list
 */
bool
is_supported_driver(adapter_t* adapter)
{
    char             path_to_pci_device[DDP_MAX_BUFFER_SIZE];
    char             driver_name[DDP_MAX_NAME_LENGTH];
    struct stat      node_attributes                         = {0};
    adapter_family_t adapter_family                          = family_none;
    uint32_t         i                                       = 0;
    bool             unsupported_device                      = FALSE;
    bool             is_supported                            = FALSE;

    memset(path_to_pci_device, '\0',DDP_MAX_BUFFER_SIZE);
    memset(driver_name, '\0',DDP_MAX_NAME_LENGTH);

    do
    {
        for(i = 0; i < unsupported_i40e_array_size; i++)
        {
            if(adapter->device_id == unsupported_i40e_device_ids[i])
            {
                debug_ddp_print("Unsupported i40e device found.\n");
                unsupported_device = TRUE;
                break;
            }
        }
        if(unsupported_device == TRUE)
        {
            break;
        }

        for(++adapter_family; adapter_family < family_last; adapter_family++)
        {
            switch(adapter_family)
            {
            case family_40G:
                strcpy_sec(driver_name, DDP_MAX_NAME_LENGTH, DDP_DRIVER_NAME_40G, strlen(DDP_DRIVER_NAME_40G));
                break;
            case family_100G:
                strcpy_sec(driver_name, DDP_MAX_NAME_LENGTH, DDP_DRIVER_NAME_100G, strlen(DDP_DRIVER_NAME_100G));
                break;
            case family_100G_SW:
                strcpy_sec(driver_name, DDP_MAX_NAME_LENGTH, DDP_DRIVER_NAME_100G_SW, strlen(DDP_DRIVER_NAME_100G_SW));
                break;
            case family_100G_SWX:
                strcpy_sec(driver_name, DDP_MAX_NAME_LENGTH, DDP_DRIVER_NAME_100G_SWX, strlen(DDP_DRIVER_NAME_100G_SWX));
                break;
            case family_none:
                /* fall-through */
            case family_last:
                /* fall-through */
            default:
                memset(driver_name, '\0', sizeof(char)*DDP_MAX_NAME_LENGTH);
                break;
            }

            if (adapter_family == family_none || adapter_family == family_last)
            {
                break;
            }

            /* check for symlink to the device file */
            snprintf(path_to_pci_device,
                     DDP_MAX_BUFFER_SIZE,
                     "%s%s/%04x:%02x:%02x.%d/",
                     PATH_TO_PCI_DRIVERS,
                     driver_name,
                     adapter->location.segment,
                     adapter->location.bus,
                     adapter->location.device,
                     adapter->location.function);

            /* use stat() instead of lstat() to follow the symlink from ../drivers into ../devices */
            stat(path_to_pci_device, &node_attributes);

            if (S_ISDIR(node_attributes.st_mode) == TRUE)
            {
                adapter->adapter_family = adapter_family;
                is_supported = TRUE;
                break;
            }
            else
            {
                continue;
            }
        }
    } while(0);

    return is_supported;
}

/* Function verifies if adapter is supported. Function checks if device is connected to the supported driver and tries
 * to match adapter with the branding string from pci.ids table. If driver is not supported or branding string was selected based on
 * two-partId match, the function tries to match device with hardcoded device table to select branding string.
 *
 * Parameters:
 * [in,out] adapter      Handle to current adapter
  *
 * Returns: TRUE if device is supported and FALSE if it is not.
 */
bool
is_device_supported(adapter_t* adapter)
{
    ddp_status_t func_status  = DDP_SUCCESS;
    match_level  match_level  = no_match;
    bool         is_supported = FALSE;

    do
    {
        is_supported = is_supported_driver(adapter);
        if(is_supported == TRUE)
        {
            func_status = get_branding_string_via_pci_ids(adapter, &match_level);
            if(func_status == DDP_SUCCESS && match_level == four_part_id_match)
            {
                debug_ddp_print("Device found in external file.\n");
                break;
            }
            if(func_status == DDP_SUCCESS && match_level == device_id_match)
            {
                debug_ddp_print("Two part id match in external file. Tool will try to match with internal table\n");
            }
            if(func_status != DDP_SUCCESS)
            {
                debug_ddp_print("Error: %d when collecting branding string from pci.ids.\n", func_status);
            }
            if(match_level < four_part_id_match)
            {
                get_brading_string_from_table(adapter, &match_level);
            }
        }
    } while(0);

    return is_supported;
}


ddp_status_t
get_connection_name(adapter_t* adapter)
{
    char           path_to_net_names[300] = {'\0'};
    struct dirent* entry                  = NULL;
    DIR*           dir                    = NULL;
    ddp_status_t   status                 = DDP_UNKNOWN_ETH_NAME;

    /* Set default value */
    strcpy_sec(adapter->connection_name,
               sizeof adapter->connection_name,
               DDP_CONNECTION_NAME_NOT_AVAILABLE,
               strlen(DDP_CONNECTION_NAME_NOT_AVAILABLE));

    snprintf(path_to_net_names,
             sizeof(path_to_net_names),
             "%s/%04x:%02x:%02x.%x/net",
             PATH_TO_SYSFS_PCI,
             adapter->location.segment,
             adapter->location.bus,
             adapter->location.device,
             adapter->location.function);

    dir = opendir(path_to_net_names);
    if(dir == NULL)
        return status;

    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_name[0] == '.')
            continue;

        strcpy_sec(adapter->connection_name,
                   sizeof adapter->connection_name,
                   entry->d_name,
                   strlen(entry->d_name));
        status = DDP_SUCCESS;
        break;
    }

    return status;
}

ddp_status_t
get_data_by_basedriver(adapter_t* adapter, ioctl_structure_t* ioctl_structure)
{
    ifreq_t      ifreq;
    ddp_status_t status            = DDP_SUCCESS;
    int          result            = 0;
    int          socket_descriptor = 0;

    memset(&ifreq, 0, sizeof ifreq);

    do
    {
        if(adapter == NULL || ioctl_structure == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
        if(socket_descriptor < 0)
        {
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            break;
        }

        if(adapter->is_virtual_function == TRUE &&
           adapter->is_usable == TRUE)
        {
            /* for virtual functions we need a connection name from PF */
            strcpy_sec(ifreq.ifr_name,
                       sizeof ifreq.ifr_name,
                       adapter->pf_connection_name,
                       strlen(adapter->pf_connection_name));
        }
        else
        {
            strcpy_sec(ifreq.ifr_name,
                       sizeof ifreq.ifr_name,
                       adapter->connection_name,
                       strlen(adapter->connection_name));
        }

        ifreq.ifr_data = (void *) ioctl_structure;

        /* Send request about data */
        ioctl_structure->command = BASEDRIVER_WRITENVM_FUNCID;
        debug_print_ioctl(ioctl_structure);
        result = ioctl(socket_descriptor, ETHTOOL_IOCTL, &ifreq);
        if(result < 0)
        {
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            debug_ddp_print("Write error! Errno: %d ", errno);
            break;
        }

        /* Received data */
        ioctl_structure->command = BASEDRIVER_READNVM_FUNCID;
        result = ioctl(socket_descriptor, ETHTOOL_IOCTL, &ifreq);
        if(result < 0)
        {
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            debug_ddp_print("Read error! Errno: %d ", errno);
        }

        debug_print_ioctl(ioctl_structure);
    } while(0);

    errno = 0;
    if(socket_descriptor >= 0)
    {
        close(socket_descriptor);
    }

    return status;
}

ddp_status_t
execute_adminq_command(adapter_t* adapter, adminq_desc_t* descriptor, uint16_t descriptor_size, uint8_t* cmd_buffer)
{
    ioctl_structure_t* ioctl_data  = NULL;
    ddp_status_t       status      = DDP_SUCCESS;
    uint16_t           buffer_size = 0;

    do
    {
        if(descriptor == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        /* preparing ioctl buffer - calculate size for data (ioctl + descriptor + adminq) */
        buffer_size = sizeof(ioctl_structure_t) + descriptor_size - 1;
        ioctl_data  = malloc_sec(buffer_size);
        if(ioctl_data == NULL || ioctl_data->data == NULL)
        {
            status = DDP_ALLOCATE_MEMORY_FAIL;
            break;
        }

        /* copy desciptor at end of ioctl_data strcuture */
        status = memcpy_sec(ioctl_data->data, descriptor_size, descriptor, descriptor_size);
        if(status != DDP_SUCCESS)
        {
            break;
        }

        /* set proper value for ioctl */
        if(adapter->is_virtual_function == TRUE && adapter->is_usable == TRUE)
        {
            ioctl_data->config    = adapter->pf_device_id << 16 | IOCTL_EXECUTE_COMMAND;
        }
        else
        {
            ioctl_data->config = adapter->device_id << 16 | IOCTL_EXECUTE_COMMAND;
        }
        ioctl_data->data_size = descriptor_size;

        status = get_data_by_basedriver(adapter, ioctl_data);
        if(status == DDP_SUCCESS)
        {
            status = memcpy_sec(descriptor, descriptor_size, ioctl_data->data, descriptor_size);
            if(status != DDP_SUCCESS)
            {
                break;
            }
        }
    } while(0);

    free(ioctl_data);
    return status;
}

ddp_status_t
write_register(adapter_t* adapter, uint32_t reg_address, uint32_t byte_number, void* input_register)
{
    ifreq_t            ifreq;
    ioctl_structure_t* ioctl_data        = NULL;
    ddp_status_t       status            = DDP_SUCCESS;
    int                result            = 0;
    int                socket_descriptor = 0;

    memset(&ifreq, 0, sizeof ifreq);

    do
    {
        /* check input parameters */
        if(adapter == NULL || input_register == NULL || byte_number == 0)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        /* check socket descriptor */
        socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
        if(socket_descriptor < 0)
        {
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            break;
        }

        /* allocate memory for ioctl structure */
        ioctl_data = malloc_sec(sizeof(ioctl_structure_t) + byte_number - 1);
        if(ioctl_data == NULL || ioctl_data->data == NULL)
        {
            status = DDP_ALLOCATE_MEMORY_FAIL;
            break;
        }

        /* set write parameters */
        ioctl_data->command   = BASEDRIVER_WRITENVM_FUNCID;
        ioctl_data->offset    = reg_address;
        ioctl_data->data_size = byte_number;
        ifreq.ifr_data        = (void*)ioctl_data;
        memcpy_sec(&ioctl_data->data[0], byte_number, (uint8_t*)input_register, byte_number);

        /* set proper value for ioctl in accordance with function type (physical/virtual) */
        if(adapter->is_virtual_function == TRUE && adapter->is_usable == TRUE)
        {
            ioctl_data->config = adapter->pf_device_id << 16 | IOCTL_REGISTER_ACCESS_COMMAND;
            strcpy_sec(ifreq.ifr_name, sizeof ifreq.ifr_name, adapter->pf_connection_name, strlen(adapter->pf_connection_name));
        }
        else
        {
            ioctl_data->config = adapter->device_id << 16 | IOCTL_REGISTER_ACCESS_COMMAND;
            strcpy_sec(ifreq.ifr_name, sizeof ifreq.ifr_name, adapter->connection_name, strlen(adapter->connection_name));
        }

        debug_print_ioctl(ioctl_data);
        /* send ioctl call */
        result = ioctl(socket_descriptor, ETHTOOL_IOCTL, &ifreq);
        if(result < 0)
        {
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            debug_ddp_print("Read error! Errno: %d\n", errno);
            break;
        }
    } while(0);

    if(socket_descriptor >= 0)
    {
        close(socket_descriptor);
    }
    free_memory(ioctl_data);

    return status;
}

ddp_status_t
read_register(adapter_t* adapter, uint32_t reg_address, uint32_t byte_number, void* output_register)
{
    ifreq_t            ifreq;
    ioctl_structure_t* ioctl_data        = NULL;
    ddp_status_t       status            = DDP_SUCCESS;
    int                result            = 0;
    int                socket_descriptor = 0;

    memset(&ifreq, 0, sizeof ifreq);

    do
    {
        /* check input parameters */
        if(adapter == NULL || output_register == NULL || byte_number == 0)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        /* check socket descriptor */
        socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
        if(socket_descriptor < 0)
        {
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            break;
        }

        /* allocate memory for ioctl structure */
        ioctl_data = malloc_sec(sizeof(ioctl_structure_t) + byte_number - 1);
        if(ioctl_data == NULL || ioctl_data->data == NULL)
        {
            status = DDP_ALLOCATE_MEMORY_FAIL;
            break;
        }

        /* set write parameters */
        ioctl_data->command   = BASEDRIVER_READNVM_FUNCID;
        ioctl_data->offset    = reg_address;
        ioctl_data->data_size = byte_number;
        ifreq.ifr_data        = (void*)ioctl_data;
        /* set proper value for ioctl in accordance with function type (physical/virtual) */
        if(adapter->is_virtual_function == TRUE && adapter->is_usable == TRUE)
        {
            ioctl_data->config = adapter->pf_device_id << 16 | IOCTL_REGISTER_ACCESS_COMMAND;
            strcpy_sec(ifreq.ifr_name, sizeof ifreq.ifr_name, adapter->pf_connection_name, strlen(adapter->pf_connection_name));
        }
        else
        {
            ioctl_data->config = adapter->device_id << 16 | IOCTL_REGISTER_ACCESS_COMMAND;
            strcpy_sec(ifreq.ifr_name, sizeof ifreq.ifr_name, adapter->connection_name, strlen(adapter->connection_name));
        }

        /* send ioctl call */
        result = ioctl(socket_descriptor, ETHTOOL_IOCTL, &ifreq);
        if(result < 0)
        {
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            debug_ddp_print("Read error! Errno: %d\n", errno);
            break;
        }

        /* copy received data to buffer */
        memcpy_sec((void*)output_register, byte_number, &ioctl_data->data[0], byte_number);
    } while(0);

    if(socket_descriptor >= 0)
    {
        close(socket_descriptor);
    }
    free_memory(ioctl_data);

    return status;
}

ddp_status_t
get_driver_info(adapter_t* adapter, driver_info_t* driver_info)
{
    ifreq_t      ifreq;
    ddp_status_t status            = DDP_SUCCESS;
    int          result            = 0;
    int          socket_descriptor = 0;

    memset(&ifreq, 0, sizeof ifreq);

    do
    {
        if(adapter == NULL || driver_info == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
        if(socket_descriptor < 0)
        {
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            break;
        }

        if(adapter->is_virtual_function == TRUE &&
           adapter->is_usable == TRUE)
        {
            /* for virtual functions we need a connection name from PF */
            strcpy_sec(ifreq.ifr_name, sizeof ifreq.ifr_name, adapter->pf_connection_name, strlen(adapter->pf_connection_name));
        }
        else
        {
            strcpy_sec(ifreq.ifr_name, sizeof ifreq.ifr_name, adapter->connection_name, strlen(adapter->connection_name));
        }


        ifreq.ifr_data = (void *) driver_info;

        /* Send request about data */
        driver_info->command = ETHTOOL_GDRVINFO;
        result = ioctl(socket_descriptor, ETHTOOL_IOCTL, &ifreq);
        if(result < 0)
        {
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            debug_ddp_print("Write error! Errno: %d ", errno);
            break;
        }

        debug_print_drvinfo(driver_info);
    } while(0);

    errno = 0;
    if(socket_descriptor >= 0)
    {
        close(socket_descriptor);
    }

    return status;
}

ddp_status_t
get_nvm_version(adapter_t* adapter, nvm_version_t* nvm_version)
{
    driver_info_t driver_info;
    char*         version_string = NULL;
    ddp_status_t  status         = DDP_SUCCESS;

    memset(&driver_info, 0, sizeof driver_info);

    do
    {
        status = get_driver_info(adapter, &driver_info);
        if(status != DDP_SUCCESS)
        {
            status = DDP_AQ_COMMAND_FAIL;
            debug_ddp_print("get_driver_info status: 0x%x\n", status);
            break;
        }

        /* Structure of firmware_version string:
         * NVM_version_major.NVM_version_minor 0xETrackID CIVD_build.CIVD_major.CIVD_minor */

        /* Get NVM version major */
        version_string = strtok(driver_info.firmware_version, ". ");
        if(version_string == NULL)
        {
            status = DDP_CANNOT_READ_DEVICE_DATA;
            break;
        }
        nvm_version->nvm_version_major = (uint8_t)(strtol(driver_info.firmware_version, &version_string, DDP_HEXADECIMAL_SYSTEM));

        /* Get NVM version minor */
        version_string = strtok(NULL, ". ");
        if(version_string == NULL)
        {
            status = DDP_CANNOT_READ_DEVICE_DATA;
            break;
        }
        nvm_version->nvm_version_minor = (uint8_t)(strtol(version_string, &version_string, DDP_HEXADECIMAL_SYSTEM));
    } while(0);

    return status;
}

ddp_status_t
initialize_tool()
{
    ddp_status_t  ddp_status = DDP_SUCCESS;

    do
    {
        if(is_root_permission() == FALSE)
        {
            ddp_status = DDP_INSUFFICIENT_PRIVILEGES;
            break;
        }

        if(check_command_parameter(DDP_XML_COMMAND_PARAMETER_BIT))
        {
            ddp_func_print_adapter_list = generate_xml;
        }
        else if (check_command_parameter(DDP_JSON_COMMAND_PARAMETER_BIT))
        {
            ddp_func_print_adapter_list = generate_json;
        }
        else
        {
            ddp_func_print_adapter_list = generate_table;
        }

        /* verify if 40G driver exists and supports DDP */
        ddp_status = i40e_verify_driver();
        if(ddp_status != DDP_SUCCESS)
        {
            debug_ddp_print("Cannot find i40e base driver!\n");
            ddp_status = DDP_SUCCESS;
        }

        /* verify if 100G driver exists - all ice drivers are expected to support DDP */
        ddp_status = ice_verify_driver();
        if(ddp_status != DDP_SUCCESS)
        {
            debug_ddp_print("Cannot find ice base driver!\n");
            ddp_status = DDP_SUCCESS;
        }

        ddp_status = ice_sw_verify_driver();
        if(ddp_status != DDP_SUCCESS)
        {
            debug_ddp_print("Cannot find ice_sw base driver!\n");
            ddp_status = DDP_SUCCESS;
        }

        ddp_status = ice_swx_verify_driver();
        if(ddp_status != DDP_SUCCESS)
        {
            debug_ddp_print("Cannot find ice_swx base driver!\n");
            ddp_status = DDP_SUCCESS;
        }
    } while(0);

    if(Global_driver_os_ctx[family_100G_SWX].driver_available == FALSE &&
       Global_driver_os_ctx[family_100G_SW].driver_available  == FALSE &&
       Global_driver_os_ctx[family_100G].driver_available     == FALSE &&
       Global_driver_os_ctx[family_40G].driver_available      == FALSE)
    {
        ddp_status = DDP_NO_BASE_DRIVER;
        debug_ddp_print("Cannot find base drivers!\n");
    }

    return ddp_status;
}

adapter_t*
get_adapter_from_list_node(node_t* node)
{
    return (adapter_t*)node->data;
}

ddp_status_t
discovery_device(adapter_t* adapter)
{
    ddp_status_t status          = DDP_SUCCESS;

    if(adapter->tdi.discovery_device != NULL)
    {
        status = adapter->tdi.discovery_device(adapter);
    }

    return status;
}

ddp_status_t
discovery_devices(list_t adapter_list)
{
    node_t*         adapter_node      = get_node(&adapter_list);
    ddp_status_t    status            = DDP_INCORRECT_FUNCTION_PARAMETERS;
    ddp_status_t    function_status   = DDP_INCORRECT_FUNCTION_PARAMETERS;
    adapter_t*      adapter           = NULL;
    adapter_t*      previous_adapter  = NULL;
    bool            is_profile_loaded = FALSE;

    while(adapter_node != NULL)
    {
        adapter = get_adapter_from_list_node(adapter_node);
        if(adapter == NULL)
        {
            status = DDP_CANNOT_READ_DEVICE_DATA;
            continue;
        }

        if(previous_adapter != NULL &&
           previous_adapter->is_usable == TRUE &&
           COMPARE_PCI_LOCATION(adapter, previous_adapter) == TRUE)
        {
            memcpy_sec(&adapter->profile_info,
                       sizeof(profile_info_t),
                       &previous_adapter->profile_info,
                       sizeof(profile_info_t));
        }
        else
        {
            function_status = discovery_device(adapter);
            if(function_status == DDP_SUCCESS && is_profile_loaded == FALSE)
            {
                is_profile_loaded = TRUE;
            }
            else if(function_status == DDP_NO_DDP_PROFILE && is_profile_loaded == TRUE)
            {
                function_status = DDP_SUCCESS;
            }
            else if(function_status != DDP_SUCCESS)
            {
                status = function_status;
            }
        }

        adapter_node = get_next_node(adapter_node);
        previous_adapter = adapter;
    }

    if(function_status == DDP_SUCCESS)
    {
        status = function_status;
    }

    return status;
}

void
print_help(void)
{
    /*      ********* 10 ****** 20 ****** 30 ****** 40 ****** 50 ****** 60 ****** 70 ******/
    printf("Usage: ddptool [parameters] [argument]\n");
    printf("    -a                  Display information about all functions for all\n"
           "                        supported devices\n");
    printf("    -h, --help, -?      Display command line help\n");
    printf("    -i DEVNAME          Display information for the specified network\n"
           "                        interface name\n");
    printf("    -j [FILENAME]       Output in JSON format to a file. If [FILENAME] is not\n"
           "                        specified, output is sent to standard output.\n");
    printf("    -l                  Silent mode\n");
    printf("    -s dddd:bb:ss.f     Display information about the device located at the\n"
           "                        specified PCI location, (where d - domain, b - bus,\n"
           "                        s - slot, f - function, all numbers are in hex)\n");
    printf("    -v                  Prints version of DDP tool\n");
    printf("    -x [FILENAME]       Output in XML format to a file. If [FILENAME] is not\n"
           "                        specified, output is sent to standard output\n");
}

void
print_header()
{
    /* for silent mode tool shouldn't print anything on screen  */
    if(check_command_parameter(DDP_SILENT_MODE_PARAMETER_BIT))
        return;

    printf("Intel(R) Dynamic Device Personalization Tool\n");
    printf("DDPTool version %d.%d.%d.%d\n",
           DDP_MAJOR_VERSION,
           DDP_MINOR_VERSION,
           DDP_BUILD_VERSION,
           DDP_FIX_VERSION);
    printf("Copyright (C) 2019 - 2021 Intel Corporation.\n\n");
}

ddp_status_t
get_device_identifier(adapter_t* adapter)
{
    ddp_status_t status = DDP_SUCCESS;

    do
    {
        /* try read data from pci config space */
        status = get_data_from_sysfs_config(adapter);
        /* if function returns SUCCESS and deviceId/vendorId are incorrect, tool needs read 4-PartId using other method */
        if(status == DDP_SUCCESS && adapter->vendor_id != 0xFFFF && adapter->device_id != 0xFFFF)
        {
            break;
        }

        get_data_from_sysfs_files(adapter);
    } while(0);

    return status;
}

ddp_status_t
initialize_adapter(adapter_t* adapter)
{
    ddp_status_t status = DDP_SUCCESS;

    switch(adapter->adapter_family)
    {
    case family_40G:
        i40e_initialize_device(adapter);
        break;
    case family_100G: /* fall-through */
    case family_100G_SW:
    case family_100G_SWX:
        ice_initialize_device(adapter);
        break;
    case family_none:
    case family_last:
        /* fall-through */
    default:
        adapter->tdi.discovery_device     = NULL;
        adapter->tdi.is_virtual_function  = NULL;
        status = DDP_INCORRECT_FUNCTION_PARAMETERS;
        break;
    }

    return status;
}

ddp_status_t
generate_adapter_list(list_t* adapter_list, char* interface_key)
{
    adapter_t       current_device;
    adapter_t       last_physical_device;
    char            location_from_dir[13];
    adapter_t*      adapter             = NULL;
    struct dirent** name_list           = NULL;
    ddp_status_t    status              = DDP_SUCCESS;
    ddp_status_t    function_status     = DDP_SUCCESS;
    int32_t         items               = 0;
    int32_t         i                   = 0;
    int             compare_result      = -1;
    int             family              = family_none;
    bool            is_vf               = FALSE;

    MEMINIT(&current_device);
    MEMINIT(&last_physical_device);
    MEMINIT(location_from_dir);

    do
    {
        items = scandir(PATH_TO_SYSFS_PCI, &name_list, 0, alphasort);

        for(i = 0; i < items; MEMINIT(&current_device), i++)
        {
            /* Get adapter PCI location */
            sscanf(name_list[i]->d_name,
                   "%04hx:%02hx:%02hx.%hx",
                   &current_device.location.segment,
                   &current_device.location.bus,
                   &current_device.location.device,
                   &current_device.location.function);

            function_status = get_device_identifier(&current_device);
            if(function_status != DDP_SUCCESS && status == DDP_SUCCESS)
            {
                debug_ddp_print("get_device_identifier error: 0x%X\n", function_status);
                status = function_status;
                continue;
            }

            if(is_device_supported(&current_device) == FALSE)
            {
                continue;
            }

            /* if the device is supported - verify if the associated driver is available/supported */
            for(family = family_none; family < family_last; family++)
            {
                if(current_device.adapter_family == family)
                {
                    if(Global_driver_os_ctx[family].driver_available == FALSE)
                    {
                        function_status = DDP_NO_BASE_DRIVER;
                        break;
                    }
                    if(Global_driver_os_ctx[family].driver_supported == FALSE)
                    {
                        function_status = DDP_UNSUPPORTED_BASE_DRIVER;
                        break;
                    }
                }
            }
            if(function_status != DDP_SUCCESS && status == DDP_SUCCESS)
            {
                status = function_status;
                continue;
            }

            /* Initialize created node */
            function_status = initialize_adapter(&current_device);
            if(function_status != DDP_SUCCESS && status == DDP_SUCCESS)
            {
                status = DDP_CANNOT_COMMUNICATE_ADAPTER;
                continue;
            }

            is_vf = is_virtual_function(&current_device);
            if(is_vf == TRUE)
            {
                if(check_command_parameter(DDP_ALL_ADAPTERS_PARAMETER_BIT) == FALSE)
                {
                    /* only with parameter -a tool works with virtual functions*/
                    continue;
                }

                current_device.is_virtual_function = TRUE;
                current_device.is_usable = FALSE; /* virtual function cannot be use for communicate with base driver */

                if(last_physical_device.is_usable == TRUE)
                {
                    strcpy_sec(current_device.pf_connection_name,
                               sizeof(current_device.pf_connection_name),
                               last_physical_device.connection_name,
                               strlen(last_physical_device.connection_name)); /* need for getting data by base driver */
                    memcpy_sec(&current_device.pf_location,
                               sizeof(current_device.pf_location),
                               &last_physical_device.location,
                               sizeof(last_physical_device.location));
                    current_device.pf_device_id = last_physical_device.device_id;
                    current_device.is_usable = TRUE; /* it's true if we have a connection name from physical function */
                }
            }

            function_status = get_connection_name(&current_device);
            if(function_status == DDP_SUCCESS)
            {
                current_device.is_usable = TRUE;
            }
            else
            {
                /* if the driver did not write connection name to sysfs
                 * we won't be able to communicate with that function */
                if(status == DDP_SUCCESS)
                {
                    status = DDP_CANNOT_COMMUNICATE_ADAPTER;
                }
                current_device.is_usable = FALSE;
                debug_ddp_print("get_connection_name error: 0x%X\n", function_status);
                /* the adapter must be added to the adapter list, so the tool cannot skip this iteration of the loop */
            }

            if(is_vf == FALSE && current_device.is_usable == TRUE)
            {
                memcpy_sec(&last_physical_device, sizeof(adapter_t), &current_device, sizeof(adapter_t));
            }

            if(check_command_parameter(DDP_LOCATION_COMMAND_PARAMETER_BIT))
            {
                memcpy_sec(location_from_dir,
                           PCI_LOCATION_STRING_SIZE,
                           name_list[i]->d_name,
                           PCI_LOCATION_STRING_SIZE);
                compare_result = strncmp(interface_key, location_from_dir, PCI_LOCATION_STRING_SIZE);
                if(compare_result != 0)
                {
                    continue;
                }
            }
            else if(check_command_parameter(DDP_INTERFACE_COMMAND_PARAMETER_BIT) &&
                    strlen(current_device.connection_name))
            {
                /* we need PF for VF */
                compare_result = strcmp(interface_key, current_device.connection_name);
                if(compare_result != 0)
                {
                    continue;
                }
            }

            adapter = malloc_sec(sizeof(adapter_t));
            if(adapter == NULL)
            {
                status = DDP_ALLOCATE_MEMORY_FAIL;
                break;
            }

            memcpy_sec(adapter, sizeof(adapter_t), &current_device, sizeof current_device);

            /* Add node to the list */
            debug_ddp_print("Adding to list device: 0x%X:0x%X:0x%X.0x%X\n",
                            adapter->location.segment,
                            adapter->location.bus,
                            adapter->location.device,
                            adapter->location.function);
            function_status = add_node_data(adapter_list, (void*) adapter, sizeof adapter);
            if(function_status != DDP_SUCCESS)
            {
                if(status == DDP_SUCCESS)
                {
                    status = function_status;
                }
                break;
            }

            if(compare_result == 0)
            {
                break; /* the provided device was found - skipping enumarete next devices */
            }
        }

        if(status == DDP_SUCCESS &&
           adapter_list->number_of_nodes == 0 &&
           (check_command_parameter(DDP_INTERFACE_COMMAND_PARAMETER_BIT) ||
            check_command_parameter(DDP_LOCATION_COMMAND_PARAMETER_BIT)))
        {
            /* Break do-while loop to protect this status from overwriting in the next if-statement */
            status = DDP_DEVICE_NOT_FOUND;
            break;
        }
        if(status == DDP_SUCCESS && adapter_list->first_node == NULL)
        {
            status = DDP_NO_SUPPORTED_ADAPTER;
        }
    } while(0);

    return status;
}

void
free_ddp_adapter_list_allocated_fields(list_t* adapter_list)
{
    node_t*    node        = get_node(adapter_list);
    node_t*    next_node   = NULL;
    adapter_t* ddp_adapter = NULL;

    while(node != NULL)
    {
        next_node = node->next_node;
        ddp_adapter = get_adapter_from_list_node(node);
        if(ddp_adapter->branding_string_allocated == TRUE)
        {
            free_memory(ddp_adapter->branding_string);
        }
        node = next_node;
    }
}

int
main(int argc, char** argv)
{
    list_t       adapter_list;
    char*        file_name       = NULL;
    char*        interface_key   = NULL;
    ddp_status_t function_status = DDP_SUCCESS;
    ddp_status_t status          = DDP_SUCCESS;

    MEMINIT(&adapter_list);
    memset(&Global_driver_os_ctx, 0, sizeof(driver_os_context_t) * family_last);

    do
    {
        function_status = parse_command_line_parameters(argc, argv, &interface_key, &file_name);

        print_header();

        if(function_status != DDP_SUCCESS)
        {
            /* header shall be print conflict parameter */
            status = function_status;
            break;
        }

        if(check_command_parameter(DDP_VERSION_COMMAND_PARAMETER_BIT))
        {
            break;
        }

        function_status = initialize_tool();
        if(function_status != DDP_SUCCESS)
        {
            debug_ddp_print("Initialize tool error: 0x%X!\n", function_status);
            status = function_status;
            break;
        }

        if(check_command_parameter(DDP_HELP_COMMAND_PARAMETER_BIT))
        {
            print_help();
            break; /* The 'Help' flag has higher priority */
        }

        function_status = generate_adapter_list(&adapter_list, interface_key);
        if(function_status != DDP_SUCCESS)
        {
            /* Do not break in case of errors - we still want to perform
             * discovery on NICs we can communicate with */
            debug_ddp_print("generate_adapter_list error: 0x%X\n", function_status);
            status = function_status;
        }
        if(adapter_list.number_of_nodes == 0)
        {
            /* Tool should not execute discovery if the device list is empty */
            break;
        }

        function_status = discovery_devices(adapter_list);
        if(function_status != DDP_SUCCESS && status == DDP_SUCCESS)
        {
            /* Do not break in case of error to print all parsed information, just keep status */
            status = function_status;
        }
    } while(0);

    /* if ddp_func_print_adapter_list is NULL set the default output */
    if(ddp_func_print_adapter_list == NULL)
    {
        ddp_func_print_adapter_list = generate_table;
    }

    status = validate_output_status(status);

    function_status = ddp_func_print_adapter_list(&adapter_list, status, file_name);
    if(function_status != DDP_SUCCESS)
    {
        status = function_status;
    }

    free_ddp_adapter_list_allocated_fields(&adapter_list);
    free_list(&adapter_list);

    return status;
}
