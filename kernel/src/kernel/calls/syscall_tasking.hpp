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

#ifndef __KERNEL_SYSCALL_TASKING__
#define __KERNEL_SYSCALL_TASKING__

#include "kernel/tasking/tasking.hpp"
#include <ghost/tasks/callstructs.h>

void syscallExit(g_task* task, g_syscall_exit* data);

void syscallExitTask(g_task* task);

void syscallYield(g_task* task, g_syscall_yield* data);

void syscallGetProcessId(g_task* task, g_syscall_get_pid* data);

void syscallGetTaskId(g_task* task, g_syscall_get_tid* data);

void syscallGetProcessIdForTaskId(g_task* task, g_syscall_get_pid_for_tid* data);

void syscallFork(g_task* task, g_syscall_fork* data);

void syscallJoin(g_task* task, g_syscall_join* data);

void syscallSleep(g_task* task, g_syscall_sleep* data);

void syscallSpawn(g_task* task, g_syscall_spawn* data);

void syscallTaskGetTls(g_task* task, g_syscall_task_get_tls* data);

void syscallProcessGetInfo(g_task* task, g_syscall_process_get_info* data);

void syscallKill(g_task* task, g_syscall_kill* data);

void syscallGetParentProcessId(g_task* task, g_syscall_get_parent_pid* data);

void syscallCreateTask(g_task* task, g_syscall_create_task* data);

void syscallGetTaskEntry(g_task* task, g_syscall_get_task_entry* data);

void syscallReleaseCliArguments(g_task* task, g_syscall_cli_args_release* data);

void syscallGetMilliseconds(g_task* task, g_syscall_millis* data);

void syscallGetNanoseconds(g_task* task, g_syscall_nanos* data);

void syscallGetExecutablePath(g_task* task, g_syscall_get_executable_path* data);

void syscallGetWorkingDirectory(g_task* task, g_syscall_get_working_directory* data);

void syscallSetWorkingDirectory(g_task* task, g_syscall_set_working_directory* data);

void syscallTaskRegisterName(g_task* task, g_syscall_task_register_name* data);

void syscallTaskAwaitByName(g_task* task, g_syscall_task_await_by_name* data);

void syscallGetTaskByName(g_task* task, g_syscall_task_get_by_name* data);

void syscallDump();

#endif
