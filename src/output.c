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

bool Global_print_debug = 0;

bool
is_debug_print_enable()
{
    return Global_print_debug;
}

void
debug_print_desc(adminq_desc_t* p_desc)
{
    printf("AdminQ:\n");
    printf("\tFlags:     0x%04X\n", p_desc->flags);
    printf("\tOpcode:    0x%04X\n", p_desc->opcode);
    printf("\tDatalen:   0x%04X\n", p_desc->datalen);
    printf("\tRetValue:  0x%04X\n", p_desc->retval);
    printf("\tcookies:   0x%08X 0x%08X\n", p_desc->cookie_low, p_desc->cookie_high);
    printf("\tparams:    0x%X 0x%X\n", p_desc->params.external.param0, p_desc->params.external.param1);
    printf("\taddr:      0x%X 0x%X\n", p_desc->params.external.addr_high, p_desc->params.external.addr_low);
    printf("-------------------------------------------------------\n");
}

void
debug_print_drvinfo(driver_info_t* driver_info)
{
    if(is_debug_print_enable() == FALSE)
        return;

    printf("DriverInfo:\n");
    printf(".command:              0x%X\n", driver_info->command);
    printf(".driver:               %s\n",   driver_info->driver);
    printf(".version:              %s\n",   driver_info->version);
    printf(".firmware_version:     %s\n",   driver_info->firmware_version);
    printf(".bus_info:             %s\n",   driver_info->bus_info);
    printf(".net_stats:            0x%X\n", driver_info->net_stats);
    printf(".test_info_length:     0x%X\n", driver_info->test_info_length);
    printf(".eeprom_dump_length:   0x%X\n", driver_info->eeprom_dump_length);
    printf(".register_dump_length: 0x%X\n", driver_info->register_dump_length);
}

void
debug_print_ioctl(ioctl_structure_t* ioctl)
{
    if(is_debug_print_enable() == FALSE)
        return;

    printf("Ioctl:\n");
    printf(".command:   0x%X\n", ioctl->command);
    printf(".config:    0x%X\n", ioctl->config);
    printf(".offset:    0x%X\n", ioctl->offset);
    printf(".data_size: 0x%X\n", ioctl->data_size);
    printf(".data:      0x%X%X%X%X\n", ioctl->data[0], ioctl->data[1], ioctl->data[2], ioctl->data[3]);
}

void
debug_ddp_print(char* format, ...)
{
    va_list args;

    /* print only in debug mode */
    if(is_debug_print_enable() == FALSE)
        return;

    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

void
ddp_print(char* format, ...)
{
    va_list args;

    /* in silent mode tool shouldn't print any information on screen */
    if(check_command_parameter(DDP_SILENT_MODE_PARAMETER_BIT))
        return;

    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

ddp_status_t
determine_output_stream(FILE** file, char* file_name)
{
    ddp_status_t status = DDP_SUCCESS;

    do
    {
        if(file_name != NULL && strlen(file_name) != 0)
        {
            *file = fopen(file_name, "w");
            if(*file == NULL)
            {
                status = DDP_CANNOT_CREATE_OUTPUT_FILE;
                break;
            }
        }
        else
        {
            *file = stdout;
        }
    } while(0);

    return status;
}

ddp_status_t
generate_table_for_file(list_t* adapter_list, ddp_status_value_t tool_status, char* file_name)
{
    char         version_string[DDP_VERSION_LENGTH];
    char         track_id_string[DDP_TRACKID_LENGTH];
    node_t*      node    = NULL;
    adapter_t*   adapter = NULL;
    ddp_status_t status  = DDP_SUCCESS;

    memset(version_string, '\0', DDP_VERSION_LENGTH * sizeof(char));
    memset(track_id_string, '\0', DDP_TRACKID_LENGTH * sizeof(char));
    
    do
    {
        /* don't print table in case of an error during file parsing. */
        if(tool_status == DDP_INCORRECT_PACKAGE_FILE || tool_status == DDP_INTERNAL_GENERIC_ERROR)
        {
            break;
        }
        node = get_node(adapter_list);
        if(node == NULL)
        {
            status = DDP_INTERNAL_GENERIC_ERROR; /* list shouldn't be empty - this is unexpected scenario */
            break;
        }
        adapter = get_adapter_from_list_node(node);

        /* If track_id is incorrect, set default strings in table */
        if(adapter->profile_info.track_id == 0)
        {
            strcpy_sec(track_id_string,
                       DDP_TRACKID_LENGTH,
                       EMPTY_MESSAGE,
                       strlen(EMPTY_MESSAGE));
        }
        else
        {
            /* Prepare string with TrackID value */
            snprintf(track_id_string,
                     DDP_TRACKID_LENGTH,
                     "%X",
                     adapter->profile_info.track_id);
        }

        /* Prepare string with Version value */
        snprintf(version_string,
                 DDP_VERSION_LENGTH,
                 "%d.%d.%d.%d",
                 adapter->profile_info.version.major,
                 adapter->profile_info.version.minor,
                 adapter->profile_info.version.update,
                 adapter->profile_info.version.draft);

        printf("File Name                          TrackId  Version      Name                  \n");
        printf("================================== ======== ============ ==============================\n");

        /* the binary file inspection mode allows to operate only on file contained one profile */
        printf("%-34s %-8s %-12s %-30s\n",
               adapter->branding_string, 
               track_id_string, 
               version_string,
               adapter->profile_info.name);
    } while(0);

    return status;
}

ddp_status_t
generate_table(list_t* adapter_list, ddp_status_value_t tool_status, char* file_name)
{
    node_t*      node    = NULL;
    adapter_t*   adapter = NULL;

    do
    {
        node = get_node(adapter_list);
        if(node == NULL)
        {
            break;
        }

        printf("NIC  DevId D:B:S.F      DevName         TrackId  Version      Name\n");
        printf("==== ===== ============ =============== ======== ============ ==============================\n");

        while(node != NULL)
        {
            adapter = get_adapter_from_list_node(node);

            print_table_adapter(adapter, stdout);

            node = get_next_node(node);
        }
    } while(0);

    return DDP_SUCCESS;
}

void
print_table_adapter(adapter_t* adapter, FILE* stream)
{

    char           version_string[DDP_VERSION_LENGTH];
    char           track_id_string[DDP_TRACKID_LENGTH];
    static uint8_t device_index                        = 0;

    memset(version_string, '\0', DDP_VERSION_LENGTH * sizeof(char));
    memset(track_id_string, '\0', DDP_TRACKID_LENGTH * sizeof(char));

    /* If device does not have any profile, set default strings in table */
    if(adapter->profile_info.section_size == 0)
    {
        strcpy_sec(track_id_string,
                   DDP_TRACKID_LENGTH,
                   EMPTY_MESSAGE,
                   strlen(EMPTY_MESSAGE));
        strcpy_sec(version_string,
                   DDP_VERSION_LENGTH,
                   EMPTY_MESSAGE,
                   strlen(EMPTY_MESSAGE));
    }
    else
    {
        /* Prepare string with TrackID value */
        snprintf(track_id_string,
                 DDP_TRACKID_LENGTH,
                 "%X",
                 adapter->profile_info.track_id);
        /* Prepare string with Version value */
        snprintf(version_string,
                 DDP_VERSION_LENGTH,
                 "%d.%d.%d.%d",
                 adapter->profile_info.version.major,
                 adapter->profile_info.version.minor,
                 adapter->profile_info.version.update,
                 adapter->profile_info.version.draft);
    }

    device_index++;
    fprintf(stream,
            "%03d) %04hX  %04hX:%02hX:%02hX.%01hX %-15s %-8s %-12s %-30s\n",
            device_index,
            adapter->device_id,
            adapter->location.segment,
            adapter->location.bus,
            adapter->location.device,
            adapter->location.function,
            adapter->connection_name,
            track_id_string,
            version_string,
            adapter->profile_info.name);
}

ddp_status_t
generate_xml(list_t* adapter_list, ddp_status_value_t tool_status, char* file_name)
{
    FILE*        xml_file = NULL;
    node_t*      node     = NULL;
    adapter_t*   adapter  = NULL;
    ddp_status_t status   = DDP_SUCCESS;

    /* Handle empty list scenario and report error */
    if(adapter_list->number_of_nodes == 0)
    {
        return generate_xml_error(tool_status, file_name, get_error_message(tool_status));
    }

    do
    {
        node = get_node(adapter_list);
        if(node == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        status = determine_output_stream(&xml_file, file_name);
        if(status != DDP_SUCCESS)
        {
            break;
        }

        fprintf(xml_file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<DDPInventory lang=\"en\">\n");
        while(node != NULL)
        {
            adapter = get_adapter_from_list_node(node);
            if(adapter == NULL)
            {
                status = DDP_CANNOT_CREATE_OUTPUT_FILE;
                break;
            }

            print_xml_adapter(adapter, xml_file);

            node = get_next_node(node);
        }
        fprintf(xml_file, "</DDPInventory>\n");
    } while(0);

    if(xml_file != NULL && file_name != NULL)
    {
        fclose(xml_file);
    }

    return status;
}

/* Function print_xml_profile() save to the input parameter stream the string with
 * information about profiles. The string is in XML format
 * 
 * Parameters:
 * [in,out] adapter      Handle to adapter
 * [out]    stream       Pointer to output buffer
 *
 * Returns: Nothing.
 */
void
print_xml_profile(adapter_t* adapter, FILE* stream)
{
    fprintf(stream,
           "\t<DDPpackage track_id=\"%08X\" version=\"%d.%d.%d.%d\" name=\"%s\"></DDPpackage>\n",
            adapter->profile_info.track_id,
            adapter->profile_info.version.major,
            adapter->profile_info.version.minor,
            adapter->profile_info.version.update,
            adapter->profile_info.version.draft,
            adapter->profile_info.name);
}

ddp_status_t
generate_xml_for_file(list_t* adapter_list, ddp_status_value_t tool_status, char* file_name)
{
    FILE*        xml_file = NULL;
    node_t*      node     = NULL;
    adapter_t*   adapter  = NULL;
    ddp_status_t status   = DDP_SUCCESS;

    /* Handle empty list scenario and report error */
    if(adapter_list->number_of_nodes == 0)
    {
        return generate_xml_error(tool_status, file_name, get_error_message(tool_status));
    }

    do
    {
        node = get_node(adapter_list);
        if(node == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        status = determine_output_stream(&xml_file, file_name);
        if(status != DDP_SUCCESS)
        {
            break;
        }

        fprintf(xml_file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<DDPInventory lang=\"en\">\n");
        adapter = get_adapter_from_list_node(node);
        if(adapter == NULL)
        {
            status = DDP_CANNOT_CREATE_OUTPUT_FILE;
            break;
        }

        fprintf(xml_file, "\t<Instance file=\"%s\">\n", adapter->branding_string);
        print_xml_profile(adapter, xml_file);
        fprintf(xml_file, "\t</Instance>\n");
        fprintf(xml_file, "</DDPInventory>\n");
    } while(0);

    if(xml_file != NULL && file_name != NULL)
    {
        fclose(xml_file);
    }

    return status;
}

void
print_xml_adapter(adapter_t* adapter, FILE* stream)
{
    fprintf(stream,
            "\t<Instance device=\"%X\" location=\"%04hX:%02hX:%02hX.%01hX\" name=\"%s\" display=\"%s\">\n",
            adapter->device_id,
            adapter->location.segment,
            adapter->location.bus,
            adapter->location.device,
            adapter->location.function,
            adapter->connection_name,
            adapter->branding_string);
    if(adapter->profile_info.section_size > 0)
    {
        print_xml_profile(adapter, stream);
    }
    fprintf(stream, "\t</Instance>\n");
}

/* The function generates XML with information discovered by the tool. Data are saved to the file provided or 
 * directly to the console if file_name is equal NULL. 
 * 
 * Parameters:
 * [in] tool_status      Tool status from previouse steps.
 * [in] file_name        File name.
 * [in] error_message    Error message to include in XML file.
 *
 * Returns: DDP_SUCCESS on success, otherwise error code. 
 */
ddp_status_t
generate_xml_error(ddp_status_value_t tool_status, char* file_name, char* error_message)
{
    FILE*        xml_file = NULL;
    ddp_status_t status   = DDP_SUCCESS;

    do
    {
        status = determine_output_stream(&xml_file, file_name);
        if(status != DDP_SUCCESS)
        {
            break;
        }

        fprintf(xml_file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<DDPInventory lang=\"en\">\n");
        fprintf(xml_file, "\t<Status result=\"failed\" error=\"%i\">%s</Status>\n", tool_status, error_message);
        fprintf(xml_file, "</DDPInventory>\n");
    } while(0);

    if(xml_file != NULL && file_name != NULL)
    {
        fclose(xml_file);
    }

    return status;
}

ddp_status_t
generate_json(list_t* adapter_list, ddp_status_value_t tool_status, char* file_name)
{
    FILE*        json_file = NULL;
    node_t*      node      = get_node(adapter_list);
    adapter_t*   adapter   = NULL;
    ddp_status_t status    = DDP_SUCCESS;

    /* Handle empty list scenario and report error */
    if(adapter_list->number_of_nodes == 0)
    {
        return generate_json_error(tool_status, file_name, get_error_message(tool_status));
    }

    do
    {
        status = determine_output_stream(&json_file, file_name);
        if(status != DDP_SUCCESS)
        {
            break;
        }

        fprintf(json_file, "{\n\t\"DDPInventory\": ");
        if(node == NULL)
        {
            fprintf(json_file, "\"null\"\n}\n");
            break;
        }

        if(adapter_list->number_of_nodes > 1)
        {
            fprintf(json_file, "[\n");
        }
        else
        {
            fprintf(json_file, "{\n");
        }

        while(node != NULL)
        {
            adapter = get_adapter_from_list_node(node);
            if(adapter == NULL)
            {
                status = DDP_CANNOT_CREATE_OUTPUT_FILE;
                break;
            }

            /* Device */
            if(adapter_list->number_of_nodes > 1)
            {
                fprintf(json_file, "\t\t{\n");
            }

            if(adapter->device_id == 0) /* If deviceid is equal 0, it means that tool is working with a file.*/
            {
                print_json_file(adapter, json_file, &(adapter_list->number_of_nodes));
            }
            else
            {
                print_json_adapter(adapter,
                                   json_file,
                                   &(adapter_list->number_of_nodes));
            }

            if(adapter_list->number_of_nodes > 1)
            {
                fprintf(json_file, "\t\t}");
            }

            node = get_next_node(node);
            if(node != NULL)
            {
                fprintf(json_file, ",\n");
                continue;
            }
            if(adapter_list->number_of_nodes > 1)
            {
                fprintf(json_file, "\n");
            }
        }
        if(adapter_list->number_of_nodes > 1)
        {
            fprintf(json_file, "\t]\n");
        }
        else
        {
            fprintf(json_file, "\t}\n");
        }
        fprintf(json_file, "}\n");
    } while(0);

    if(json_file != NULL && file_name != NULL)
    {
        fclose(json_file);
    }

    return status;
}

void
print_json_file(adapter_t* adapter, FILE* stream, uint32_t* number_of_nodes)
{
    char* indentation_string = "\t\t";

    if(*number_of_nodes > 1)
    {
        indentation_string = "\t\t\t";
    }

    /* Print file name */

    fprintf(stream, "%s\"file_name\": \"%s\"\n", indentation_string, adapter->branding_string);
    /* Print DDP profile */
    fprintf(stream, "%s\"DDPpackage\": {\n", indentation_string);
    fprintf(stream,
            "%s\t\"track_id\": \"%X\",\n",
            indentation_string,
            adapter->profile_info.track_id);
    fprintf(stream,
            "%s\t\"version\": \"%d.%d.%d.%d\",\n",
            indentation_string,
            adapter->profile_info.version.major,
            adapter->profile_info.version.minor,
            adapter->profile_info.version.update,
            adapter->profile_info.version.draft);
    fprintf(stream,
            "%s\t\"name\": \"%s\"\n",
            indentation_string,
            adapter->profile_info.name);
    fprintf(stream, "%s}\n", indentation_string);
}

void
print_json_adapter(adapter_t* adapter, FILE* stream, uint32_t* number_of_nodes)
{
    char* indentation_string = "\t\t";

    if(*number_of_nodes > 1)
    {
        indentation_string = "\t\t\t";
    }

    fprintf(stream, "%s\"device\": \"%X\",\n", indentation_string, adapter->device_id);
    fprintf(stream,
            "%s\"address\": \"%04hX:%02hX:%02hX.%01hX\",\n",
            indentation_string,
            adapter->location.segment,
            adapter->location.bus,
            adapter->location.device,
            adapter->location.function);
    fprintf(stream, "%s\"name\": \"%s\",\n", indentation_string, adapter->connection_name);
    fprintf(stream, "%s\"display\": \"%s\"", indentation_string, adapter->branding_string);

    /* Skip last comma if there will be no DDP profile section */
    adapter->profile_info.section_size > 0 ? fprintf(stream, ",\n") : fprintf(stream, "\n");

    /* Print DDP profile */
    if(adapter->profile_info.section_size > 0)
    {
        fprintf(stream, "%s\"DDPpackage\": {\n", indentation_string);
        fprintf(stream,
                "%s\t\"track_id\": \"%X\",\n",
                indentation_string,
                adapter->profile_info.track_id);
        fprintf(stream,
                "%s\t\"version\": \"%d.%d.%d.%d\",\n",
                indentation_string,
                adapter->profile_info.version.major,
                adapter->profile_info.version.minor,
                adapter->profile_info.version.update,
                adapter->profile_info.version.draft);
        fprintf(stream,
                "%s\t\"name\": \"%s\"\n",
                indentation_string,
                adapter->profile_info.name);
        fprintf(stream, "%s}\n", indentation_string);
    }
}

ddp_status_t
generate_json_error(ddp_status_value_t tool_status, char* file_name, char* error_message)
{
    FILE*        json_file = NULL;
    ddp_status_t status    = DDP_SUCCESS;

    do
    {
        status = determine_output_stream(&json_file, file_name);
        if(status != DDP_SUCCESS)
        {
            break;
        }

        fprintf(json_file, "{\"DDPInventory\":{\n");
        fprintf(json_file, "\t\t\"error\": \"%i\"\n", tool_status);
        fprintf(json_file, "\t\t\"message\": \"%s\"\n", error_message);
        fprintf(json_file, "\t}\n");
        fprintf(json_file, "}\n");

    } while(0);

    if(json_file != NULL && file_name != NULL)
    {
        fclose(json_file);
    }

    return status;
}
