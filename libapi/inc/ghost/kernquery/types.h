/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *  Ghost, a micro-kernel based operating system for the x86 architecture    *
 *  Copyright (C) 2015, Max Schlüssel <lokoxe@gmail.com>                     *
 *                                                                           *
 *  This program is free software: you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation, either version 3 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef GHOST_API_KERNQUERY_TYPES
#define GHOST_API_KERNQUERY_TYPES

#include "../common.h"
#include "../tasks.h"
#include "../filesystem.h"
#include "../stdint.h"

__BEGIN_C

/**
 *
 */
typedef int g_kernquery_status;
#define G_KERNQUERY_STATUS_SUCCESSFUL ((g_kernquery_status) 0)
#define G_KERNQUERY_STATUS_UNKNOWN_ID ((g_kernquery_status) 1)
#define G_KERNQUERY_STATUS_ERROR ((g_kernquery_status) 2)

/**
 * Command IDs
 */
#define G_KERNQUERY_TASK_COUNT 0x600
#define G_KERNQUERY_TASK_LIST 0x601
#define G_KERNQUERY_TASK_GET_BY_ID 0x602

/**
 * Used in the {G_KERNQUERY_TASK_COUNT} query to retrieve the number
 * of existing tasks.
 */
typedef struct
{
	uint32_t count;
} __attribute__((packed)) g_kernquery_task_count_data;

/**
 * Used in the {G_KERNQUERY_TASK_LIST} query to retrieve a list that
 * contains the id of each existing task.
 */
typedef struct
{
	g_tid* id_buffer;
	uint32_t id_buffer_size;

	uint32_t filled_ids;
} __attribute__((packed)) g_kernquery_task_list_data;

/**
 * Used in the {G_KERNQUERY_TASK_GET_BY_ID} query to retrieve
 * information about a specific task.
 */
typedef struct
{
	g_tid id;
	uint8_t found;

	g_tid parent;
	g_task_type type;
	char identifier[512];
	char source_path[G_PATH_MAX];

	g_virtual_address memory_used;
	uint64_t cpu_time;
} __attribute__((packed)) g_kernquery_task_get_data;

__END_C

#endif
