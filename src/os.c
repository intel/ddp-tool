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

#include "os.h"

#define PATH_BUFFER_SIZE 300

/* Function strcat_sec() concatenates two strings by copying up to chars_to_cpy characters
 * from src buffer (but no more than up to the 1st null terminator) and adding them
 * after dest_from characters in destination buffer. Null terminator is added at the end
 * of the concatenated string. This function performs checks on passed offsets and the
 * length of the buffers contents. The underlying functions for strcat_sec() are
 * strncat() and strncpy().
 *
 * Parameters:
 * [in,out] dest         Buffer to which the concatenated string will be added to
 * [in]     dest_from    Offset at which the src string will be added in dest
 * [in]     dest_size    Size of destination buffer
 * [in]     src          Buffer from which characters will be read and added at the
 *                       end of dest buffer's content
 * [in]     chars_to_cpy Size of the source buffer (number of chars to be copied into
 *                       dest)
 *
 * Returns: Pointer to destination buffer (dest) or NULL if either the parameters
 *          passed are incorrect or no characters have been copied due to constrain
 *          checks.
 */
char*
strcat_sec(char* dest, uint32_t dest_from, uint32_t dest_size, const char* src, uint32_t chars_to_cpy)
{
    char*    result    = NULL;
    uint32_t total_len = 0;
    uint32_t src_len   = 0;
    uint32_t dest_len  = 0;

    do
    {
        if(dest == NULL || dest_from == 0 || dest_size ==0 || src == NULL || chars_to_cpy == 0)
        {
            debug_ddp_print("Error: incorrect parameters for concatenation.");
            break;
        }

        dest_len = strlen(dest);
        if((dest_from >= dest_size) || (dest_len >= dest_size))
        {
            debug_ddp_print("Error: incorrect dest buffer size provided.");
            break;
        }

        if(dest_from > dest_len)
        {
            /* we add chars from src no further than at the end
             * of existing string in dest buffer */
            dest_from = dest_len;
            debug_ddp_print("Warning: destination offset is greater than current string in dest buffer.");
        }

        src_len = strlen(src);
        if(chars_to_cpy > src_len)
        {
            /* we don't want to allow copying anything from src after the
             * null terminator because strncpy() is not optimized for that */
            chars_to_cpy = src_len;
            debug_ddp_print("Warning: source offset is greater than current string in src buffer.");
        }

        total_len = chars_to_cpy + dest_from;
        if(total_len >= dest_size)
        {
            /* -1 is for the null-terminator added by
             * strncpy() at strlen(dest) + chars_to_cpy */
            chars_to_cpy = dest_size - dest_from -1;
            if(chars_to_cpy <= 0)
            {
                debug_ddp_print("Error: Dest buffer full.");
                break;
            }
        }

        result = strncat(dest,src,chars_to_cpy);
    } while(0);

    return result;
}

ddp_status_t
memcpy_sec(void* destination, uint32_t destination_size, void* source, uint32_t source_size)
{
    ddp_status_t status           = DDP_SUCCESS;
    uint32_t     safe_buffer_size = 0;

    do
    {
        if(destination == NULL || source == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        if(destination_size < source_size)
        {
            safe_buffer_size = destination_size;
        }
        else
        {
            safe_buffer_size = source_size;
        }

        memcpy(destination, source, safe_buffer_size);
    } while(0);

    return status;
}

ddp_status_t
strcpy_sec(char* destination, uint32_t destination_size, char* source)
{
    ddp_status_t status      = DDP_SUCCESS;
    uint32_t     source_size = 0;

    do
    {
        if(destination == NULL || source == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        source_size = strlen(source);

        if(destination_size < source_size)
        {
            status = DDP_INCORRECT_BUFFER_SIZE;
            break;
        }

        strncpy(destination, source, destination_size - 1);
    } while(0);

    return status;
}

void*
malloc_sec(size_t size)
{
    void* buffer = NULL;

    do
    {
        /* check if allocation make sens */
        if(size == 0)
            break;

        /* allocate memory */
        buffer = malloc(size);
        if(buffer == NULL)
            break;
        memset(buffer, 0, size);
    } while(0);

    return buffer;
}

bool
is_root_permission()
{
    if(getuid() != 0)
        return FALSE;

    return TRUE;
}

uint16_t
get_uint16_t_from_file(char* path)
{
    FILE*    file    = NULL;
    char     text[7] = {'\0'};
    uint16_t value   = 0;
    int      status  = 0;

    do
    {
        file = fopen(path, "r");
        if(file == NULL)
            break;

        status = fscanf(file, "%6s", text);
        if(status != 1)
            break;

        value = (uint16_t) strtol(text, NULL, 16);
    } while(0);

    if(file != NULL)
    {
        fclose(file);
    }

    return value;
}

void
get_data_from_sysfs_files(adapter_t* adapter)
{
    char         path_to_device[PATH_BUFFER_SIZE]      = {'\0'};
    char         path_to_device_file[PATH_BUFFER_SIZE] = {'\0'};

    snprintf(path_to_device,
             sizeof(path_to_device),
             "%s/%04x:%02x:%02x.%d/",
             PATH_TO_SYSFS_PCI,
             adapter->location.segment,
             adapter->location.bus,
             adapter->location.device,
             adapter->location.function);

    /* get vendor_id */
    memcpy_sec(path_to_device_file, sizeof path_to_device_file, path_to_device, sizeof path_to_device);
    strcat_sec(path_to_device_file, strlen(path_to_device_file), PATH_BUFFER_SIZE, "vendor", strlen("vendor"));
    adapter->vendor_id = get_uint16_t_from_file(path_to_device_file);

    /* get dev_id */
    memcpy_sec(path_to_device_file, sizeof path_to_device_file, path_to_device, sizeof path_to_device);
    strcat_sec(path_to_device_file, strlen(path_to_device_file), PATH_BUFFER_SIZE, "device", strlen("device"));
    adapter->device_id = get_uint16_t_from_file(path_to_device_file);

    /* get subven_id */
    memcpy_sec(path_to_device_file, sizeof path_to_device_file, path_to_device, sizeof path_to_device);
    strcat_sec(path_to_device_file, strlen(path_to_device_file), PATH_BUFFER_SIZE, "subsystem_vendor", strlen("subsystem_vendor"));
    adapter->subvendor_id = get_uint16_t_from_file(path_to_device_file);

    /* get subsys_id */
    memcpy_sec(path_to_device_file, sizeof path_to_device_file, path_to_device, sizeof path_to_device);
    strcat_sec(path_to_device_file, strlen(path_to_device_file), PATH_BUFFER_SIZE, "subsystem_device", strlen("subsystem_device"));
    adapter->subdevice_id = get_uint16_t_from_file(path_to_device_file);
}

ddp_status_t
get_data_from_sysfs_config(adapter_t* adapter)
{
    char               path_to_config_file[300] = {'\0'};
    pci_config_space_t pci_config_space;
    FILE*              config                   = NULL;
    ddp_status_t       status                   = DDP_SUCCESS;
    uint16_t           length                   = 0;
    uint8_t            i                        = 0;
    size_t             read_size                = 0;

    MEMINIT(&pci_config_space);

    do
    {
        snprintf(path_to_config_file,
                 sizeof(path_to_config_file),
                 "%s/%04x:%02x:%02x.%d/config",
                 PATH_TO_SYSFS_PCI,
                 adapter->location.segment,
                 adapter->location.bus,
                 adapter->location.device,
                 adapter->location.function);

        config = fopen(path_to_config_file, "rb");
        if(config == NULL)
        {
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            break;
        }

        /* get file size */
        fseek(config, 0, SEEK_END);
        length = ftell(config);

        if(length < sizeof pci_config_space)
        {
            status = DDP_CANNOT_COMMUNICATE_ADAPTER;
            break;
        }

        rewind(config);

        for(i = 0; i < sizeof pci_config_space; i++)
        {
            read_size = fread((uint8_t*) (&pci_config_space) + i, 1, 1, config);
            if(read_size != 1)
            {
                status = DDP_CANNOT_COMMUNICATE_ADAPTER;
                break;
            }
        }
        if(status != DDP_SUCCESS)
        {
            break;
        }

        adapter->vendor_id = pci_config_space.vendor_id;
        adapter->device_id = pci_config_space.device_id;
        adapter->subdevice_id = pci_config_space.subsystem_device_id;
        adapter->subvendor_id = pci_config_space.subsystem_vendor_id;
    } while(0);

    if(config != NULL)
        fclose(config);

    return status;
}

ddp_status_t
get_driver_version_from_os(driver_os_version_t* driver_version, char* version_file_path)
{
    char         line[32]          = {'\0'};
    char*        result            = NULL;
    FILE*        file              = NULL;
    int          sscanf_status     = 0;
    ddp_status_t ddp_status        = DDP_SUCCESS;

    do
    {
        file = fopen(version_file_path, "r");
        if(file == NULL)
        {
            ddp_status = DDP_NO_BASE_DRIVER;
            break;
        }

        result = fgets(line, MAX_DRIVER_VERSION_STRING_LENTGH, file);
        if(result == NULL)
        {
            ddp_status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        sscanf_status = sscanf(line,
                               "%hu%*[.]%hu%*[.]%hu",
                               &driver_version->major,
                               &driver_version->minor,
                               &driver_version->build);
        if(sscanf_status != NUMBER_OF_DRIVER_VERSION_PARTS)
        {
            ddp_status = DDP_UNSUPPORTED_BASE_DRIVER;
        }
    } while(0);

    if(file != NULL)
    {
        fclose(file);
    }

    return ddp_status;
}