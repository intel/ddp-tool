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

#include "os.h"
#include <ctype.h>
#include <stdlib.h>

#define PATH_BUFFER_SIZE 300

/* NOTE: all _sec() functions are intended as "all or nothing":
 * i.e. if all requested chars/bytes cannot be copied the operation should be abandoned
 * however chars beyond the 1st null terminator in source buffer shall not be processed */

/* extended_strcpy_sec()
 * copies declared number of characters from source buffer into destination buffer
 * starting at declared offset in destination buffer
 */
char*
extended_strcpy_sec(char* dest, uint32_t dest_from, uint32_t dest_size, const char* src, uint32_t chars_to_copy)
{
    char*    result    = NULL;
    uint32_t src_len   = 0;
    uint32_t dest_len  = 0;

    do
    {
        if(dest == NULL || dest_size == 0 || src == NULL || chars_to_copy == 0)
        {
            debug_ddp_print("Error: incorrect parameters for extended string copy.\n");
            break;
        }

        /* offset cannot be outside dest buffer */
        if(dest_from >= dest_size)
        {
            debug_ddp_print("Error: incorrect offset for dest buffer provided.\n");
            break;
        }

        /* string copied from source into dest at given offset must not exceed dest size */
        if((dest_from + chars_to_copy) >= dest_size)
        {
            debug_ddp_print("Error: requested number of characters to copy from src will not fit into dest buffer.\n");
            break;
        }

        /* do not add chars in dest past the existing null terminator */
        dest_len = strlen(dest);
        if(dest_from > dest_len)
        {
            dest_from = dest_len;
            debug_ddp_print("Warning: destination offset is greater than current string in dest buffer.\n");
        }

        /* do not copy from src after null terminator */
        src_len = strlen(src);
        if(chars_to_copy > src_len)
        {
            chars_to_copy = src_len;
            debug_ddp_print("Warning: source offset is greater than current string in src buffer.\n");
        }

        result = strncpy((dest + dest_from),src,chars_to_copy);
        dest[dest_from + chars_to_copy] = '\0';
    } while(0);

    return result;
}


/* strcat_sec()
 * safe wrapper for strncat()
 *
 * checks:
 * - arguments passed (null/0 checks)
 * - number of characters to copy from source vs source size
 *   (do not copy from src beyond the 1st null terminator)
 * - dest size vs declared dest size
 *   (make sure this is a concatenation not overwriting)
 * - if src + dest strings fit in declared dest buffer size
 *
 * does not check:
 * - dest and src overlap
 * - strncat() result
 * - if dest and src contain valid strings
 *
 * returns: strcat result (pointer to dest) or NULL if strcat of any of the checks fail
 */
char*
strcat_sec(char* dest, uint32_t dest_size, const char* src, uint32_t chars_to_copy)
{
    char*    result    = NULL;
    uint32_t src_len   = 0;
    uint32_t dest_len  = 0;

    do
    {
        if(dest == NULL || src == NULL || chars_to_copy == 0)
        {
            debug_ddp_print("Error: incorrect parameters for concatenation.\n");
            break;
        }

        /* length of string in destination buffer cannot be greater then declared destination buffer size */
        dest_len = strlen(dest);
        if(dest_len > dest_size)
        {
            debug_ddp_print("Error: incorrect dest buffer size provided.\n");
            break;
        }

        /* do not allow copying from src past null terminator */
        src_len = strlen(src);
        if(chars_to_copy > src_len)
        {
            debug_ddp_print("Warning: declared number of characters to copy goes beyond the string in source buffer.\n");
            chars_to_copy = src_len;
        }

        /* check if src string fits into dest buffer - chars to copy must be 1 less then dest size or strncat will not null terminate */
        if((dest_len + chars_to_copy) >= dest_size)
        {
            debug_ddp_print("Error: src string will not fit into dest buffer.");
            break;
        }

        result = strncat(dest,src,chars_to_copy);
        dest[dest_len + chars_to_copy] = '\0';
    } while(0);

    return result;
}

ddp_status_t
memcpy_sec(void* destination, uint32_t destination_size, void* source, uint32_t source_size)
{
    ddp_status_t status           = DDP_SUCCESS;

    do
    {
        if(destination == NULL || source == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        if(destination_size < source_size)
        {
            debug_ddp_print("Error: Destination buffer too small for memory copy operation\n.");
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        memcpy(destination, source, source_size);
    } while(0);

    return status;
}

ddp_status_t
strcpy_sec(char* destination, uint32_t destination_size, const char* source, uint32_t chars_to_copy)
{
    ddp_status_t status      = DDP_SUCCESS;
    uint32_t     source_size = 0;

    do
    {
        if(destination == NULL || source == NULL || chars_to_copy == 0)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        source_size = strlen(source);

        /* do not allow copying from src past null terminator */
        if(chars_to_copy > source_size)
        {
            debug_ddp_print("Warning: declared number of characters to copy goes beyond the string in source buffer.\n");
            chars_to_copy = source_size;
        }

        /* abandon operation if string from source cannot fit into destination */
        if(destination_size <= chars_to_copy)
        {
            status = DDP_INCORRECT_BUFFER_SIZE;
            break;
        }

        strncpy(destination, source, chars_to_copy);
        destination[chars_to_copy] = '\0';
    } while(0);

    return status;
}

void*
malloc_sec(size_t size)
{
    void* buffer = NULL;

    do
    {
        /* check if allocation make sense */
        if(size == 0)
        {
            break;
        }

        /* allocate memory */
        buffer = malloc(size);
        if(buffer == NULL)
        {
            break;
        }

        memset(buffer, 0, size);
    } while(0);

    return buffer;
}

bool
is_root_permission()
{
    if(getuid() != 0)
    {
        return FALSE;
    }

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
        {
            break;
        }

        status = fscanf(file, "%6s", text);
        if(status != 1)
        {
            break;
        }

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
    char         path_to_device[PATH_BUFFER_SIZE];
    char         path_to_device_file[PATH_BUFFER_SIZE];

    MEMINIT(&path_to_device);
    MEMINIT(&path_to_device_file);

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
    strcat_sec(path_to_device_file, PATH_BUFFER_SIZE, "vendor", strlen("vendor"));
    adapter->vendor_id = get_uint16_t_from_file(path_to_device_file);

    /* get dev_id */
    memcpy_sec(path_to_device_file, sizeof path_to_device_file, path_to_device, sizeof path_to_device);
    strcat_sec(path_to_device_file, PATH_BUFFER_SIZE, "device", strlen("device"));
    adapter->device_id = get_uint16_t_from_file(path_to_device_file);

    /* get subven_id */
    memcpy_sec(path_to_device_file, sizeof path_to_device_file, path_to_device, sizeof path_to_device);
    strcat_sec(path_to_device_file, PATH_BUFFER_SIZE, "subsystem_vendor", strlen("subsystem_vendor"));
    adapter->subvendor_id = get_uint16_t_from_file(path_to_device_file);

    /* get subsys_id */
    memcpy_sec(path_to_device_file, sizeof path_to_device_file, path_to_device, sizeof path_to_device);
    strcat_sec(path_to_device_file, PATH_BUFFER_SIZE, "subsystem_device", strlen("subsystem_device"));
    adapter->subdevice_id = get_uint16_t_from_file(path_to_device_file);
}

ddp_status_t
get_data_from_sysfs_config(adapter_t* adapter)
{
    pci_config_space_t pci_config_space;
    char               path_to_config_file[300];
    FILE*              config                   = NULL;
    ddp_status_t       status                   = DDP_SUCCESS;
    uint16_t           length                   = 0;
    uint8_t            i                        = 0;
    size_t             read_size                = 0;

    MEMINIT(&path_to_config_file);
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

    /* TO_DO: add support for driver version with -k suffix */

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

/* Functions looks up 2-PartId/4-PartId in pci.ids and collects a branding string for current adapter
 *
 * Parameters:
 * [in, out] adapter     ddp adapter struct - branding string written to adapter->branding_string
 * [out]    match_level  4 if 4-PartId match, 2 if 2-PartId match, 0 if no match
 *
 * Returns: success if no issues or error code otherwise
 *
 * Note: pci.ids structure:
 * vendor_id[space][space]string[\n]
 * [tab]device_id[space][space]string[\n]
 * [tab][tab]subvendor_id[space]subdevice_id[space][space]string[\n]
 *
 * Example from pci.ids (each 4 leading spaces are \t)
8086  Intel Corporation
    0d58  Ethernet Controller XXV710 Intel(R) FPGA Programmable Acceleration Card N3000 for Networking
        8086 0000  Ethernet Controller XXV710 Intel(R) FPGA Programmable Acceleration Card N3000 for Networking
        8086 0001  Ethernet Controller XXV710 Intel(R) FPGA Programmable Acceleration Card N3000 for Networking
    1000  82542 Gigabit Ethernet Controller (Fiber)
        0e11 b0df  NC6132 Gigabit Ethernet Adapter (1000-SX)
        0e11 b0e0  NC6133 Gigabit Ethernet Adapter (1000-LX)
        0e11 b123  NC6134 Gigabit Ethernet Adapter (1000-LX)
        1014 0119  Netfinity Gigabit Ethernet SX Adapter
        8086 1000  PRO/1000 Gigabit Server Adapter
    1001  82543GC Gigabit Ethernet Controller (Fiber)
        0e11 004a  NC6136 Gigabit Server Adapter
        1014 01ea  Netfinity Gigabit Ethernet SX Adapter
        8086 1002  PRO/1000 F Server Adapter
        8086 1003  PRO/1000 F Server Adapter
 */
ddp_status_t
get_branding_string_via_pci_ids(adapter_t* ddp_adapter, match_level* match_level)
{
    FILE*        file_handle          = NULL;
    const char*  path_to_pci_ids      = "/usr/share/hwdata/pci.ids";
    ddp_status_t ddp_status           = DDP_SUCCESS;
    uint32_t     vendor_id_string_len = 0;
    uint32_t     candidate_id         = 0;
    uint32_t     base_hex             = 16;
    bool         stop_search          = FALSE;

    /* buffers */
    char         branding_string[DDP_MAX_BRANDING_SIZE];
    char         line[DDP_MAX_BUFFER_SIZE];
    char         id_candidate[DDP_MAX_BUFFER_SIZE];

    /* offsets */
    uint32_t     space_offset            = 1; /* 1 char for 1 space */
    uint32_t     double_space_offset     = 2; /* 2 chars for 2 spaces */
    uint32_t     id_offset               = 4; /* 4 chars of vendor id */
    uint32_t     id_spaces_offset        = id_offset + double_space_offset; /* offset by vendor id (4) + 2 spaces */
    uint32_t     sub_id_offsets          = (2*id_offset) + (3*space_offset); /* 8 chars for sub ids + 3 spaces */

    /* initialize buffers */
    memset(branding_string, '\0',DDP_MAX_BRANDING_SIZE);
    memset(line, '\0', DDP_MAX_BUFFER_SIZE);
    memset(id_candidate, '\0', DDP_MAX_BUFFER_SIZE);

    *match_level = no_match;

    do
    {
        file_handle = fopen(path_to_pci_ids, "r");
        if(file_handle == NULL)
        {
            ddp_status = DDP_FILE_ACCESS_ERROR;
            break;
        }

        while(fgets(line, DDP_MAX_BUFFER_SIZE, file_handle))
        {
            if(stop_search == TRUE)
            {
                break;
            }

            switch(*match_level)
            {
                /* looking for vendor id match */
                case no_match:
                {
                    /* vendor id is a top level key so the 1st char must be an alphanumeric */
                    if(isalnum(line[0]) == 0)
                    {
                        continue; /* read next line */
                    }
                    else
                    {
                        /* collect candidate vendor id */
                        strcpy_sec(id_candidate, DDP_MAX_BUFFER_SIZE, line, id_offset);
                        candidate_id = (uint32_t)(strtol(id_candidate, NULL /* end pointer */, base_hex));

                        /* compare candidate vendor id with adapter vendor id */
                        if(candidate_id == ddp_adapter->vendor_id)
                        {
                            /* keep this value in case subvendor/subdevice id match */
                            vendor_id_string_len = strlen(line + id_spaces_offset);
                            /* initialize branding string */
                            strcat_sec(branding_string,
                                       DDP_MAX_BRANDING_SIZE,
                                       line + id_spaces_offset,
                                       vendor_id_string_len);
                            /* vendor id found */
                            *match_level = vendor_id_match;
                        }
                    }
                    break;
                }

                /* looking for device id match */
                case vendor_id_match:
                {
                    /* if the 1st char of the line is an alphanumeric - we have reached a new vendor id section
                     * the search is over - in a correct pci.ids file there is only a single unique vendor id section */
                    if(isalnum(line[0]) != 0)
                    {
                        stop_search = TRUE;
                        continue; /* jump to next while condition check to break there */
                    }
                    /* device id is preceded by a single tab */
                    if(line[0] != '\t' || isalnum(line[1]) == 0)
                    {
                        /* some lines will start with # acting as a comment - skip those */
                        continue;
                    }
                    else
                    {
                        /* collect candidate device id */
                        strcpy_sec(id_candidate,
                                   DDP_MAX_BUFFER_SIZE,
                                   line + *match_level, /* offset of 1 tab */
                                   id_offset /* 4 chars of device id */);
                        candidate_id = (uint32_t)(strtol(id_candidate, NULL /* end pointer */, base_hex));

                        /* compare candidate device id with adapter device id */
                        if(candidate_id == ddp_adapter->device_id)
                        {
                            /* extend branding string */
                            strcat_sec(branding_string,
                                       DDP_MAX_BRANDING_SIZE,
                                       line + *match_level + id_spaces_offset /* offset by tab (1) + device id (4) + 2 spaces */,
                                       strlen(line + *match_level + id_spaces_offset) /* offset by tab (1) + device id (4) + 2 spaces */);
                            /* device id found */
                            *match_level = device_id_match;
                        }
                    }
                    break;
                }

                /* looking for subvendor and subdevice id match */
                case device_id_match:
                {
                    /* if the 1st char of the line is an alphanumeric - we have reached a new vendor id section
                     * the search is over - in a correct pci.ids file there is only a single unique vendor id section */
                    if(isalnum(line[0]) != 0)
                    {
                        stop_search = TRUE;
                        continue; /* jump to next while condition check to break there */
                    }
                    /* if the 1st char is a tab, but the 2nd is an alphanumeric - we're in a new device id section
                     * and the search is over */
                    if((line[0] == '\t') && isalnum(line[1]) != 0)
                    {
                        stop_search = TRUE;
                        continue; /* jump to next while condition check to break there */
                    }
                    /* subvendor id is preceded by two tabs and starts with an alphanumeric*/
                    if(line[0] != '\t' || line[1] != '\t' || isalnum(line[2]) == 0)
                    {
                        /* some lines will start with # acting as a comment - skip those */
                        continue;
                    }
                    else
                    {
                        /* collect candidate subvendor id */
                        strcpy_sec(id_candidate,
                                   DDP_MAX_BUFFER_SIZE,
                                   line + *match_level, /* offset of 2 tabs */
                                   id_offset /* 4 chars of subvendor id */);
                        candidate_id = (uint32_t)(strtol(id_candidate, NULL /* end pointer */, base_hex));

                        /* compare candidate subvendor id with adapter subvendor id */
                        if(candidate_id == ddp_adapter->subvendor_id)
                        {
                            /* collect candidate subdevice id */
                            strcpy_sec(id_candidate,
                                       DDP_MAX_BUFFER_SIZE,
                                       line + *match_level + id_offset + space_offset, /* offset of 2 tabs + subvendor id (4) + space (1) */
                                       id_offset /* 4 chars of subdevice id */);
                            candidate_id = (uint32_t)(strtol(id_candidate, NULL /* end pointer */, base_hex));
                            /* compare candidate subdevice id with adapter subdevice id */
                            if(candidate_id == ddp_adapter->subdevice_id)
                            {
                                /* 4-part id match -> concat vendor id string with subvendor/subdevice id string
                                 * writing over the 2-part id match string */
                                extended_strcpy_sec(branding_string,
                                                    vendor_id_string_len,
                                                    DDP_MAX_BRANDING_SIZE,
                                                    line + *match_level + sub_id_offsets, /* 2 tabs, 8 chars, 3 spaces */
                                                    strlen(line + *match_level + sub_id_offsets) /* 2 tabs, 8 chars, 3 spaces*/);
                                *match_level = four_part_id_match;
                                stop_search = TRUE;
                            }
                        }
                    }
                    break;
                }
                case four_part_id_match:
                {
                    break;
                }
            }
        }

        /* set branding string in adapter structure */
        ddp_adapter->branding_string = (char*)(malloc(DDP_MAX_BRANDING_SIZE * sizeof(char)));
        if(ddp_adapter->branding_string == NULL)
        {
            ddp_status = DDP_ALLOCATE_MEMORY_FAIL;
            break;
        }
        ddp_adapter->branding_string_allocated = TRUE;
        if(*match_level == device_id_match || *match_level == four_part_id_match)
        {
            /* process branding string from pci.ids */
            replace_character(branding_string, '\n', ' ' ); /* remove new line characters */
            branding_string[strlen(branding_string) - 1] = '\0'; /* remove trailing white space */
            strcpy_sec(ddp_adapter->branding_string,
                       DDP_MAX_BRANDING_SIZE,
                       branding_string,
                       strlen(branding_string));
        }
        else
        {
            if(ddp_adapter->adapter_family == family_40G)
            {
                snprintf(ddp_adapter->branding_string,
                         DDP_MAX_BRANDING_SIZE,
                         "Unrecognized device on %04X:%02X:%02X.%d using %s driver",
                         ddp_adapter->location.segment,
                         ddp_adapter->location.bus,
                         ddp_adapter->location.device,
                         ddp_adapter->location.function,
                         DDP_DRIVER_NAME_40G);
                ddp_status = DDP_MATCH_ERROR;
            }
            if(ddp_adapter->adapter_family == family_100G)
            {
                snprintf(ddp_adapter->branding_string,
                         DDP_MAX_BRANDING_SIZE,
                         "Unrecognized device on %04X:%02X:%02X.%d using %s driver",
                         ddp_adapter->location.segment,
                         ddp_adapter->location.bus,
                         ddp_adapter->location.device,
                         ddp_adapter->location.function,
                         DDP_DRIVER_NAME_100G);
                ddp_status = DDP_MATCH_ERROR;
            }
            *match_level = 0; /* neither 2 nor 4-part id match found */
        }
    } while(0);

    if(file_handle != NULL)
    {
        fclose(file_handle);
    }

    return ddp_status;
}

/* do not pass immutable strings to buffer argument */
char*
replace_character(char* buffer, char find, char replace)
{
    char *current = NULL;
    while((current = strchr(buffer, find)) != NULL)
    {
        *current = replace;
    }
    return buffer;
}
