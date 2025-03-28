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

#include "stdio.h"
#include "stdio_internal.h"

FILE* __open_file_list = 0;
g_user_mutex open_file_list_lock = 0;

void __open_file_list_initialize()
{
	open_file_list_lock = g_mutex_initialize();
}

void __open_file_list_add(FILE* file)
{
	__open_file_list_lock();

	if (__open_file_list)
	{
		__open_file_list->prev = file;
		file->next = __open_file_list;
	}
	__open_file_list = file;

	__open_file_list_unlock();
}

void __open_file_list_remove(FILE* file)
{
	__open_file_list_lock();

	if(file == __open_file_list)
	{
		__open_file_list = file->next;
	}
	if(file->prev)
	{
		file->prev->next = file->next;
	}
	if(file->next)
	{
		file->next->prev = file->prev;
	}

	__open_file_list_unlock();
}

void __open_file_list_lock()
{
	g_mutex_acquire(open_file_list_lock);
}

void __open_file_list_unlock()
{
	g_mutex_release(open_file_list_lock);
}
