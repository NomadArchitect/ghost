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

#ifndef GHOST_API_KERNQUERY_CALLSTRUCTS
#define GHOST_API_KERNQUERY_CALLSTRUCTS

#include "../common.h"
#include "../stdint.h"
#include "types.h"

__BEGIN_C

/**
 * @field command
 * 		query command
 *
 * @field buffer
 * 		communication buffer
 *
 * @field status
 * 		one of the {g_kernquery_status} status codes
 */
typedef struct {
	uint16_t command;
	uint8_t* buffer;
	g_kernquery_status status;
}__attribute__((packed)) g_syscall_kernquery;

__END_C

#endif
