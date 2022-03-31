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

#include "ddp_types.h"
#include "ddp_list.h"

node_t*
get_next_node(node_t* node)
{
    if(node != NULL)
    {
        return node->next_node;
    }

    return NULL;
}

void
free_list(list_t* list)
{
    node_t* node      = get_node(list);
    node_t* next_node = NULL;

    while(node != NULL)
    {
        next_node = node->next_node;
        free_memory(node->data);
        free_memory(node);
        node = next_node;
    }
}

node_t*
get_node(list_t* list)
{
    if(list == NULL)
    {
        return NULL;
    }

    return list->first_node;
}

node_t*
get_last_node(list_t* list)
{
    node_t* node = NULL;

    node = get_node(list);

    while(node != NULL && node->next_node != NULL)
    {
        node = node->next_node;
    }

    return node;
}

ddp_status_t
add_node_data(list_t* list, void* node, size_t size)
{
    node_t*      new_node  = NULL;
    node_t*      last_node = NULL;
    ddp_status_t status    = DDP_SUCCESS;

    do
    {
        if(list == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        new_node = malloc_sec(sizeof(node_t));
        if(new_node == NULL)
        {
            status = DDP_ALLOCATE_MEMORY_FAIL;
            break;
        }

        list->number_of_nodes++;

        last_node = get_last_node(list);
        if(last_node == NULL)
        {
            list->first_node = new_node;
            new_node->data = node;
            new_node->data_size = size;
            break;
        }

        /* save data and size to new item*/
        new_node->data = node;
        new_node->data_size = size;

        /* set new node as new element on list */
        last_node->next_node = new_node;
    } while(0);

    return status;
}

ddp_status_t
remove_current_node(list_t* list, void* node)
{
    node_t*      previous_node = NULL;
    node_t*      current_node  = NULL;
    ddp_status_t status        = DDP_SUCCESS;

    do
    {
        if(list == NULL || node == NULL)
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        current_node = get_node(list);
        while(current_node != NULL)
        {
            if(current_node == node) /* Pointer addresses comparison */
            {
                break;
            }
            previous_node = current_node;
            current_node = get_next_node(current_node);
        }
        if(current_node == NULL) /* This is the end of the list */
        {
            status = DDP_INCORRECT_FUNCTION_PARAMETERS;
            break;
        }

        if(previous_node != NULL) /* If matched node is not the first on the list */
        {
            previous_node->next_node = current_node->next_node;
        }
        else
        {
            list->first_node = current_node->next_node;
        }

        /* Remove node from the list */
        free_memory(current_node->data);
        free_memory(current_node);
        list->number_of_nodes--;
    } while(0);

    return status;
}
