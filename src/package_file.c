/****************************************************************************************
* Copyright (C) 2021 Intel Corporation
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

#include "package_file.h"
#include <errno.h>
#include <string.h>
#include <output.h>

/* The function reads content of file. The content is save to the buffer or 
 * if buffer is null, the buffer_size is set to size of file. 
 * 
 * Parameters:
 *  [in] file_handle - handle to the file
 *  [in, out] buffer - buffer dedicatet for content of file
 *  [in, out] buffer_size - size of buffer
 *
 * Returns: DDP_SUCCESS on success, otherwise error code. 
 */
ddp_status_t
ddp_read_file(FILE* file_handle, char* buffer, uint64_t* buffer_size)
{
    ddp_status_t status    = DDP_SUCCESS;
    uint64_t     file_size = 0;

    do
    {
        if(file_handle == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        /* get file size */
        fseek(file_handle, 0, SEEK_END);
        file_size = ftell(file_handle);
        if(buffer == NULL)
        {
            *buffer_size = file_size;
            break;
        }

        /* get file content */
        fseek(file_handle, 0, SEEK_SET);
        file_size = fread(buffer, sizeof(char), *buffer_size, file_handle);
        if(file_size != *buffer_size)
        {
            status = DDP_INCORRECT_BUFFER_SIZE;
            break;
        }
    } while (0);

    return status;
}

/* Function opens the binary file to read.
 *
 * Parameters:
 *  [in] input_file_name - file name
 *
 * Returns: Handle to file
 */
FILE*
ddp_open_file(char* input_file_name)
{
    FILE* file_handle = NULL;

    do
    {
        /* validate input parameters */
        if(input_file_name == NULL || strlen(input_file_name) == 0)
        {
            break;
        }

        file_handle = fopen(input_file_name, "rb");
        if(file_handle == NULL)
        {
            debug_ddp_print("Cannot open file errno: %d\n", errno);
        }
    } while (0);

    return file_handle;
}

/* The function closes the file.
 *
 * Parameters:
 *  [in] file_handle - handle to the file
 *
 * Returns: Nothing
 */
void
ddp_close_file(FILE** file_handle)
{
    if(file_handle != NULL && *file_handle != NULL)
    {
        fclose(*file_handle);
        *file_handle = NULL;
    }
}

/* Function verifies if provided buffer can be used for DDP Package inspection 
 * 
 * Parameters:
 *  [in] buffer      - Content of the DDP package file
 *  [in] buffer_size - Size of buffer (DDP Package file size) 
 *
 * Returns: DDP_SUCCESS on success othwerise error code
 */
ddp_status_t
validate_package_file(char* buffer, uint64_t buffer_size)
{
    ddp_status_t status = DDP_SUCCESS;
    package_header_t* pkg_header = (package_header_t*)buffer;    
    uint32_t i = 0;

    do
    {
        /* buffer needs to contain the package header */
        if(buffer == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        /* Buffer needs to have enough size to store following data:
         *  - package header
         *  - array with segments offsets and segment headers for each segment declared in package header
         */
        if(buffer_size <  sizeof(package_header_t) ||
           buffer_size <= sizeof(package_header_t) + 
                          (pkg_header->entries_number * sizeof(pkg_header->segment_offset) + 
                           pkg_header->entries_number * sizeof(segment_header_t))
          )
        {
            status = DDP_INCORRECT_PACKAGE_FILE;
            break;
        }

        debug_ddp_print("Package info:\n");
        debug_ddp_print(" version: 0x%X\n", pkg_header->version);
        debug_ddp_print(" entries_number: 0x%X\n", pkg_header->entries_number);
        /* Validate each offset in segment array. The buffer should have space for segment header for each segment 
           provided in offsets array */
        for(i = 0; i < pkg_header->entries_number; i++)
        {
            uint32_t package_header_size = sizeof(package_header_t) + pkg_header->entries_number * sizeof(pkg_header->segment_offset);
            if(pkg_header->segment_offset[i] < package_header_size)
            {
                status = DDP_INCORRECT_PACKAGE_FILE;
                debug_ddp_print("Incorrect pacakge header. Segments cannot be located at offset 0.\n");
                break;
            }
            if(pkg_header->segment_offset[i] + sizeof(segment_header_t) > buffer_size)
            {
                status = DDP_INCORRECT_PACKAGE_FILE;
                debug_ddp_print("Incorrect pacakge header. There is no space for segment pointed by offset %d\n", i);
                break;
            }
        }
    } while(0);

    return status;
}

/* Function iterates through buffer to get segments from package file. Operation is applicable
 * as long as buffer contains segments. If pointer to the last segments was returned, 
 * the function returns NULL in next iteration. Function uses data from package header to 
 * recognize number of segments and discover the specific segment offset.
 * 
 * Parameters:
 *  [in] buffer      - Content of the DDP package file, must be cast to the package_header_t
 *  [in] buffer_size - Size of buffer (DDP Package file size) 
 *
 * Returns: Buffer to the segment header or NULL 
 */
segment_header_t*
get_next_segment_from_package(char* buffer, uint64_t buffer_size)
{
    static uint8_t    segment_number = 0;
    package_header_t* pkg_header = (package_header_t*)buffer;
    segment_header_t* segment = NULL;
    uint64_t          segment_offset = 0;

    do
    {
        /* Buffer should contain: Package header (8 bytes) + segment table (N*4 bytes) + segment (44 bytes).*/
        if(buffer == NULL || buffer_size < sizeof(package_header_t) + sizeof(pkg_header->entries_number) + sizeof(segment_header_t))
        {
            break;
        }

        if(segment_number >= pkg_header->entries_number)
        {
            break;
        }

        segment_offset = pkg_header->segment_offset[segment_number];
        /* Corruption in this case should be catch in function 'validate_package_header' but we don't know if this
           function was called correctly */
        if(segment_offset + sizeof(segment_header_t) > buffer_size)
        {
            break;
        }

        segment = (segment_header_t*)(buffer + segment_offset);
        debug_ddp_print("segment[%d] at offset 0x%lX\n", segment_number, segment_offset);
    } while(0);

    if(segment == NULL)
    {
        segment_number = 0;
    }
    else
    {
        segment_number++;
    }

    return segment;
}

/* The function validates metadata in the ICE segment
 * 
 * Parameters:
 *  [in] segment_header_t    - pointer to segment header in the package buffer
 *
 * Returns: Returns: DDP_SUCCESS on success, otherwise error code. 
 */
ddp_status_t
validate_metadata_in_ice_segment(segment_header_t* segment)
{
    ice_segment_header_t* ice_config_header = (ice_segment_header_t*) segment;
    ddp_status_t status = DDP_SUCCESS;

    do
    {
        if(segment == NULL || segment->type != DDP_SEGMENT_TYPE_ICE_CONFIGURATION)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        if(ice_config_header->generic_header.size < sizeof(ice_buff_header_t) + sizeof(ice_section_entry_t))
        {
            debug_ddp_print("Size 0x%lX is to small. Move to the next segment\n",
            sizeof(ice_buff_header_t) + sizeof(ice_section_entry_t));
            status = DDP_INCORRECT_SEGMENT;
        }
    } while(0);

    return status;
}

/* Function finds and returns a metada section in the ICE configuration segment. 
 * 
 * Parameters:
 *  [in] segment_header_t    - pointer to segment header in the package buffer
 *
 * Returns: pointer to the buffer with the metada.
 */
ice_config_section_metadata_t*
get_ice_metadata_section_from_segment(segment_header_t* segment)
{
    ice_segment_header_t* ice_config_header = (ice_segment_header_t*) segment;
    ice_config_section_metadata_t* metadata_section = NULL;
    ice_buff_array_t* buff_array = NULL;
    ice_buff_header_t* buff_header = NULL;
    ice_section_entry_t* section_entry = NULL;
    ice_buf_table_t* ice_buff = NULL;
    uint8_t buffer_index = 0;
    uint8_t section_index = 0;

    if(segment == NULL || 
       segment->type != DDP_SEGMENT_TYPE_ICE_CONFIGURATION || 
       segment->size < sizeof(ice_segment_header_t))
    {
        debug_ddp_print("Incorrect segment header.\n");
        return NULL;
    }

    /* Config section should contains devices array, ice_buf_table*/
    if(segment->size < sizeof(ice_segment_header_t) + 
                       ice_config_header->device_table_count * sizeof(ice_package_device_entry_t) +
                       sizeof(ice_buf_table_t)
      )
    {
        debug_ddp_print("Incorrect ice configuration segment.\n");
    }

    /* ICE buffer header is located just after device array */
    ice_buff = (ice_buf_table_t*)(ice_config_header->device) + 
                                  ice_config_header->device_table_count * sizeof(ice_package_device_entry_t) + 1;
    buff_array = ice_buff->buff_array;

    while(buffer_index < ice_buff->buf_count)
    {
        buff_header   = (ice_buff_header_t*)(buff_array + buffer_index);
        section_entry = (ice_section_entry_t*) buff_header->section_entry;

        for(section_index = 0; section_index < buff_header->section_count; section_index++)
        {
            debug_ddp_print("section entry: \n\ttype: 0x%X \toffset: 0x%X \tsize: 0x%X\n",
                            section_entry[section_index].type,
                            section_entry[section_index].offset,
                            section_entry[section_index].size);
            if(section_entry[section_index].type == DDP_SECTION_TYPE_METADATA)
            {
                metadata_section = (ice_config_section_metadata_t*)((uint8_t*)buff_header + section_entry[section_index].offset);
                break;
            }
        }
        if(metadata_section != NULL)
        {
            break;
        }
        buffer_index++;
    }

    return metadata_section;
}

/* The function validate the segment header
 * 
 * Parameters:
 *  [in] segment_header_t - pointer to segment header in the package buffer
 *
 * Returns: Returns: DDP_SUCCESS on success, otherwise error code. 
 */
ddp_status_t
validate_segment_header(segment_header_t* segment)
{
    ddp_status_t status = DDP_SUCCESS;

    do
    {
        if(segment == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        debug_ddp_print(" type: 0x%X\n", segment->type);
        debug_ddp_print(" version: 0x%X\n", segment->version);
        debug_ddp_print(" size: 0x%X\n", segment->size);
        debug_ddp_print(" name: %s\n", segment->name);

        if(segment->size <= sizeof(segment_header_t))
        {
            status = DDP_INCORRECT_SEGMENT;
            break;
        }
    } while(0);

    return status;
}

/* Macro copies data from metadata from file to the adapter structure.
 * 
 * Parameters:
 *  [in, out] adapter  - Handle to adapter
 *  [in]      metadata - Pointer to metadata read from file
 *
 * Returns: Nothing.
 */
#define COPY_PACKAGE_INFO_FROM_METADATA(adapter, metadata) ((adapter->profile_info.track_id)  = (metadata->trackid)); \
                                                           ((adapter->profile_info.version)   = (metadata->version)); \
                                                           memcpy_sec((adapter->profile_info.name),                   \
                                                                      sizeof((adapter->profile_info.name)),           \
                                                                      (metadata->name),                               \
                                                                      DDP_MAX_ICE_PROFILE_NAME_LENGTH * sizeof(char))

/* The function adds to the adapter_list a single item to store information read from the file. 
 * This item does not represent any physical device, it is just used only to store information read from files. 
 * 
 * Parameters:
 *  [in, out] adapter_list    - pointer to the initalized list.
 *  [in]      input_file_name - file name to analyze
 *
 * Returns: Returns: DDP_SUCCESS on success, otherwise error code. 
 */
ddp_status_t
analyze_binary_file(list_t* adapter_list, char* input_file_name)
{
    uint64_t                       buffer_size       = 0;
    ice_config_section_metadata_t* metadata_section  = NULL;
    adapter_t*                     adapter           = NULL;
    segment_header_t*              segment_header    = NULL;
    global_metadata_t*             global_metadata   = NULL;
    FILE*                          file_handle       = NULL;
    char*                          buffer            = NULL;
    ddp_status_t                   status            = DDP_SUCCESS;
    package_type_t                 package_type      = package_none;

    do
    {
        /* validate input parameters */
        if(input_file_name == NULL || strlen(input_file_name) == 0 || adapter_list == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        adapter = get_adapter_from_list_node(get_node(adapter_list));

        /* open file */
        file_handle = ddp_open_file(input_file_name);
        if(file_handle == NULL)
        {
            debug_ddp_print("Cannot open file!\n");
            status = DDP_INTERNAL_GENERIC_ERROR;
            break;
        }

        /* get file size */
        status = ddp_read_file(file_handle, buffer, &buffer_size);
        if(status != DDP_SUCCESS)
        {
            debug_ddp_print("ddp_read_file error: 0x%X\n", status);
            break;
        }

        /* allocate buffor for file content */
        buffer = malloc_sec(buffer_size);
        if(buffer == NULL)
        {
            debug_ddp_print("malloc sec error!");
            status = DDP_ALLOCATE_MEMORY_FAIL;
            break;
        }

        /* read file */
        status = ddp_read_file(file_handle, buffer, &buffer_size);
        if(status != DDP_SUCCESS)
        {
            debug_ddp_print("ddp_read_file error 0x%X\n", status);
        }

        /* release resource immediately after use */
        ddp_close_file(&file_handle);

        adapter->branding_string = input_file_name;

        status = validate_package_file(buffer, buffer_size);
        if(status != DDP_SUCCESS)
        {
            debug_ddp_print("Incorrect DDP Package file.\n");
            break;
        }

        do
        {
            segment_header = get_next_segment_from_package(buffer, buffer_size);
            if(segment_header == NULL)
            {
                break;
            }

            status = validate_segment_header(segment_header);
            if(status != DDP_SUCCESS)
            {
                break;
            }

            /* Intel 700 series Global Metadata Segment structure is not compatible with Intel 800 as fields "Package Name" and 
             * "Track ID" are swapped. Due to this, function needs to recognize the target device for this package as a first.
             * This information is stored in segment type in the package file:
             * type == 0x10 - target device is 800 Series
             * type == 0x11 - target device is 700 Series
             * Related of the profile type, the trackId is stored in different location
             */
            if(segment_header->type == DDP_SEGMENT_TYPE_ICE_CONFIGURATION)
            {
                /* information should be read from Metadata section of the ICE Configuration segment*/
                package_type = package_ice;
                debug_ddp_print("This is profile for Intel 800 series.\n");
                status = validate_metadata_in_ice_segment(segment_header);
                if(status == DDP_INCORRECT_FUNCTION_PARAMETERS)
                {
                    break;
                }
                else if(status != DDP_SUCCESS)
                {
                    /* try to find ICE segment with metadata */
                    continue;
                }

                metadata_section = get_ice_metadata_section_from_segment(segment_header);
                if(metadata_section != NULL)
                {
                    COPY_PACKAGE_INFO_FROM_METADATA(adapter, metadata_section);
                }
            }
            else if(segment_header->type == DDP_SEGMENT_TYPE_I40E_CONFIGURATION)
            {
                /* information should be read from global metadata segment */
                package_type = package_i40e;
                debug_ddp_print("This is profile for Intel 700 series.\n");
                if(global_metadata != NULL)
                {
                    /* global metadata was found in one of the previous step, let use this adapter and break the loop */
                    COPY_PACKAGE_INFO_FROM_METADATA(adapter, global_metadata);
                    break;
                }
                /*don't break! let's find the generic Metadata segment */
                continue;
            }
            else if(segment_header->type == DDP_SEGMENT_TYPE_GLOBAL_METADATA)
            {
                global_metadata = (global_metadata_t*)segment_header->data;
                if(package_type == package_i40e)
                {
                    /* save data to adapter strucutre and break the loop */
                    COPY_PACKAGE_INFO_FROM_METADATA(adapter, global_metadata);
                    break;
                }
            }
        } while(segment_header != NULL);

        if(package_type == package_none)
        {
            status = DDP_INCORRECT_PACKAGE_FILE;
        }
    } while(0);

    /* release resources and buffer */
    ddp_close_file(&file_handle);
    free(buffer);

    return status;
}
