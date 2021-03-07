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

#include "cmdparams.h"

static uint32_t static_command_line_parameters = 0;
static char*           static_char_options = "s:ahlj::i:x::v?";
static struct option   static_string_options[] =
{
    {"help",  0, 0,    'h'},
    {"help",  0, 0,    '?'},
    {NULL,    0, NULL, 0}
};

bool
check_command_parameter(uint32_t param)
{
    return (static_command_line_parameters & param) ? TRUE : FALSE;
}

bool
is_character_printable(char character)
{
    bool is_printable = FALSE;

    if(character >= ' ' && character <= '~')
    {
        is_printable = TRUE;
    }

    return is_printable;
}

bool
is_string_printable(char* string)
{
    uint32_t character_index = 0;
    uint32_t string_size     = 0;
    bool     is_printable    = FALSE;

    do
    {
        if(string == NULL)
        {
            break;
        }

        string_size = strlen(string);
        if(string_size == 0)
        {
            break;
        }

        for(character_index = 0; character_index < string_size; character_index++)
        {
            is_printable = is_character_printable(string[character_index]);
            if(is_printable == FALSE)
            {
                break;
            }
        }
    } while(0);

    return is_printable;
}

void
convert_to_lowercase(char* string)
{
    uint32_t character_index = 0;
    uint32_t string_length   = 0;

    if(string != NULL)
    {
        string_length = strlen(string);

        for(character_index = 0; character_index < string_length; character_index++)
        {
            string[character_index] = tolower(string[character_index]);
        }
    }
}

ddp_status_t
validate_command_line_parameters(int argc, char** argv)
{
    char*        current_parameter  = NULL;
    const char*  known_parameter    = NULL;
    ddp_status_t status             = DDP_SUCCESS;
    uint32_t     option_index       = 0;
    uint32_t     parameter_index    = 0;
    uint32_t     parameter_length   = 0;
    bool         is_parameter_found = FALSE;

    for(parameter_index = 1; parameter_index < argc; parameter_index++)
    {
        /* Prepare variables for the next iteration */
        current_parameter  = argv[parameter_index];
        is_parameter_found = FALSE;

        /* Check if current parameter typed by user is not NULL, empty or non-printable */
        if(current_parameter == NULL || is_string_printable(current_parameter) == FALSE)
        {
            status = DDP_BAD_COMMAND_LINE_PARAMETER;
            break;
        }

        parameter_length = strlen(current_parameter);
        if(parameter_length > DDP_CMD_LINE_MIN_LONG_PARAMETER_SIZE &&
           current_parameter[0] == '-'                             &&
           current_parameter[1] == '-')
        {
            /* Go through options structure to find long parameter typed by user */
            known_parameter = static_string_options[option_index].name;
            while(known_parameter != NULL && is_parameter_found != TRUE)
            {
                /* Skip leading dashes */
                current_parameter += DDP_CMD_LINE_DOUBLE_DASH_PREFIX_SIZE;
                if(strcmp(current_parameter, known_parameter) == 0)
                {
                    is_parameter_found = TRUE;
                    break;
                }
                option_index++;
                known_parameter = static_string_options[option_index].name;
            }
            if(is_parameter_found == FALSE)
            {
                /* This case means that parameter was called with double dash and
                 * parameter is not present in the structure with long parameters.
                 * Double dash is reserved for long parameters.
                 * e.g. ./ddptool --test ; ./ddptool --xml etc. */
                status = DDP_BAD_COMMAND_LINE_PARAMETER;
                break;
            }
        }
        else if(parameter_length > DDP_CMD_LINE_MIN_SHORT_PARAMETER_SIZE &&
                current_parameter[0] == '-')
        {
            /* This case means that parameter was called with single dash and
             * parameter is longer than one character - one dash is reserved
             * for short parameters.
             * e.g. ./ddptool -xds ; ./ddptool -ft etc. */
            status = DDP_BAD_COMMAND_LINE_PARAMETER;
            break;
        }
    }

    return status;
}

ddp_status_t
parse_command_line_parameters(int argc, char** argv, char** interface_key, char** file_name)
{
    ddp_status_t status       = DDP_SUCCESS;
    int          parameter    = 0;
    int          string_index = 0;

    do
    {
        status = validate_command_line_parameters(argc, argv);
        if(status != DDP_SUCCESS)
        {
            break;
        }

        /* Do not print default getopt_long function error messages */
        opterr = 0;

        while(1)
        {
            parameter = getopt_long(argc,
                                    argv,
                                    static_char_options,
                                    static_string_options,
                                    &string_index);
            if(parameter == -1)
            {
                break;
            }

            if(optopt != 0)
            {
                /* If optopt is not zero (it contains invalid parameter),
                 * stop parsing parameters */
                status = DDP_BAD_COMMAND_LINE_PARAMETER;
                break;
            }
            switch(parameter)
            {
            case DDP_ALL_ADAPTERS_PARAMETER:
                status = CHECK_DUPLICATE(DDP_ALL_ADAPTERS_PARAMETER_BIT);
                static_command_line_parameters |= DDP_ALL_ADAPTERS_PARAMETER_BIT;
                break;
            case DDP_LOCATION_COMMAND_PARAMETER:
                status = CHECK_DUPLICATE(DDP_LOCATION_COMMAND_PARAMETER_BIT);
                if(optarg == NULL)
                {
                    status = DDP_BAD_COMMAND_LINE_PARAMETER;
                    break;
                }
                *interface_key = optarg;
                convert_to_lowercase(*interface_key);
                static_command_line_parameters |= DDP_LOCATION_COMMAND_PARAMETER_BIT;
                break;
            case DDP_INTERFACE_COMMAND_PARAMETER:
                status = CHECK_DUPLICATE(DDP_INTERFACE_COMMAND_PARAMETER_BIT);
                if(optarg == NULL)
                {
                    status = DDP_BAD_COMMAND_LINE_PARAMETER;
                    break;
                }
                *interface_key = optarg;
                static_command_line_parameters |= DDP_INTERFACE_COMMAND_PARAMETER_BIT;
                break;
            case DDP_HELP1_COMMAND_PARAMETER:
            case DDP_HELP2_COMMAND_PARAMETER:
                status = CHECK_DUPLICATE(DDP_HELP_COMMAND_PARAMETER_BIT);
                static_command_line_parameters |= DDP_HELP_COMMAND_PARAMETER_BIT;
                break;
            case DDP_XML_COMMAND_PARAMETER:
                status = CHECK_DUPLICATE(DDP_XML_COMMAND_PARAMETER_BIT);
                if(optarg != NULL)
                {
                    *file_name = optarg;
                }
                else if(argv[optind] != NULL && argv[optind][0] != '-')
                {
                    *file_name = argv[optind];
                    optind++;
                }
                static_command_line_parameters |= DDP_XML_COMMAND_PARAMETER_BIT;
                break;
            case DDP_VERSION_COMMAND_PARAMETER:
                status = CHECK_DUPLICATE(DDP_VERSION_COMMAND_PARAMETER_BIT);
                static_command_line_parameters |= DDP_VERSION_COMMAND_PARAMETER_BIT;
                break;
            case DDP_SILENT_MODE:
                status = CHECK_DUPLICATE(DDP_SILENT_MODE_PARAMETER_BIT);
                static_command_line_parameters |= DDP_SILENT_MODE_PARAMETER_BIT;
                break;
            case DDP_JSON_COMMAND_PARAMETER:
                status = CHECK_DUPLICATE(DDP_JSON_COMMAND_PARAMETER_BIT);
                if(optarg != NULL)
                {
                    *file_name = optarg;
                }
                else if(argv[optind] != NULL && argv[optind][0] != '-')
                {
                    *file_name = argv[optind];
                    optind++;
                }
                static_command_line_parameters |= DDP_JSON_COMMAND_PARAMETER_BIT;
                break;
            default:
                status = DDP_BAD_COMMAND_LINE_PARAMETER;
                break;
            }

            if(CONFLICT_PARAMETERS(DDP_LOCATION_COMMAND_PARAMETER_BIT, DDP_INTERFACE_COMMAND_PARAMETER_BIT) ||
               CONFLICT_PARAMETERS(DDP_XML_COMMAND_PARAMETER_BIT, DDP_JSON_COMMAND_PARAMETER_BIT))
            {
                status = DDP_BAD_COMMAND_LINE_PARAMETER;
            }

            if(status != DDP_SUCCESS)
            {
                break;
            }
        }
    } while(0);

    return status;
}
