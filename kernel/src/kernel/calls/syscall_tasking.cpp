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

#include "kernel/calls/syscall_tasking.hpp"
#include "kernel/filesystem/filesystem_process.hpp"
#include "kernel/memory/memory.hpp"
#include "kernel/tasking/user_mutex.hpp"
#include "kernel/system/interrupts/interrupts.hpp"
#include "kernel/tasking/tasking_directory.hpp"
#include "kernel/tasking/clock.hpp"
#include "kernel/utils/wait_queue.hpp"
#include "shared/logger/logger.hpp"
#include "shared/utils/string.hpp"

void syscallSleep(g_task* task, g_syscall_sleep* data)
{
	mutexAcquire(&task->lock);
	clockWaitForTime(task->id, clockGetLocal()->time + data->milliseconds);
	task->status = G_TASK_STATUS_WAITING;
	mutexRelease(&task->lock);
	taskingYield();
}

void syscallYield(g_task* task)
{
	taskingYield();
}

void syscallExit(g_task* task, g_syscall_exit* data)
{
	waitQueueWake(&task->waitersJoin);
	taskingProcessKillAllTasks(task->process->id);
	taskingExit();
}

void syscallExitTask(g_task* task)
{
	taskingExit();
}

void syscallGetProcessId(g_task* task, g_syscall_get_pid* data)
{
	data->id = taskingGetCurrentTask()->process->id;
}

void syscallGetTaskId(g_task* task, g_syscall_get_tid* data)
{
	data->id = taskingGetCurrentTask()->id;
}

void syscallGetProcessIdForTaskId(g_task* task, g_syscall_get_pid_for_tid* data)
{
	g_task* theTask = taskingGetById(data->tid);
	if(theTask)
	{
		data->pid = theTask->process->id;
	}
	else
	{
		data->pid = G_PID_NONE;
	}
}

void syscallJoin(g_task* task, g_syscall_join* data)
{
	mutexAcquire(&task->lock);
	taskingWaitForExit(data->taskId, task->id);
	task->status = G_TASK_STATUS_WAITING;
	mutexRelease(&task->lock);
	taskingYield();
}

void syscallSpawn(g_task* task, g_syscall_spawn* data)
{
	g_fd fd;
	g_fs_open_status open = filesystemOpen(data->path, G_FILE_FLAG_MODE_READ, task, &fd);
	if(open == G_FS_OPEN_SUCCESSFUL)
	{
		auto target = taskingSpawn(fd, G_SECURITY_LEVEL_APPLICATION);
		data->status = target.status;
		data->validationDetails = target.validation;

		filesystemClose(task->process->id, fd, true);

		if(data->status == G_SPAWN_STATUS_SUCCESSFUL)
		{
			data->pid = target.process->id;
			filesystemProcessCreateStdio(task->process->id, data->inStdio, target.process->id, data->outStdio);

			target.process->environment.executablePath = stringDuplicate(data->path);
			if(data->args)
				target.process->environment.arguments = stringDuplicate(data->args);
			if(data->workdir)
				target.process->environment.workingDirectory = stringDuplicate(data->workdir);

			target.process->main->status = G_TASK_STATUS_RUNNING;
		}
	}
	else
	{
		data->status = G_SPAWN_STATUS_IO_ERROR;
		logInfo("%! failed to find binary '%s'", "kernel", data->path);
	}
}

void syscallTaskGetTls(g_task* task, g_syscall_task_get_tls* data)
{
	data->userThreadLocal = task->threadLocal.userThreadLocal;
}

void syscallProcessGetInfo(g_task* task, g_syscall_process_get_info* data)
{
	data->processInfo = task->process->userProcessInfo;
}

void syscallKill(g_task* task, g_syscall_kill* data)
{
	g_task* target = taskingGetById(data->pid);
	if(target)
	{
		data->status = G_KILL_STATUS_SUCCESSFUL;
		target->process->main->status = G_TASK_STATUS_DEAD;
		waitQueueWake(&target->waitersJoin);
		taskingYield();
	}
	else
	{
		data->status = G_KILL_STATUS_NOT_FOUND;
	}
}

void syscallCreateTask(g_task* task, g_syscall_create_task* data)
{
	mutexAcquire(&task->process->lock);

	g_task* newTask = taskingCreateTask((g_virtual_address) data->initialEntry, task->process,
	                                    task->process->main->securityLevel);
	if(newTask)
	{
		newTask->userEntry.function = data->userEntry;
		newTask->userEntry.data = data->userData;
		data->threadId = newTask->id;
		data->status = G_CREATE_TASK_STATUS_SUCCESSFUL;
		taskingAssignBalanced(newTask);
	}
	else
	{
		data->status = G_CREATE_TASK_STATUS_FAILED;
	}

	mutexRelease(&task->process->lock);
}

void syscallGetTaskEntry(g_task* task, g_syscall_get_task_entry* data)
{
	data->userData = task->userEntry.data;
	data->userEntry = task->userEntry.function;
}

void syscallFork(g_task* task, g_syscall_fork* data)
{
	logInfo("syscall not implemented: fork");
}

void syscallGetParentProcessId(g_task* task, g_syscall_get_parent_pid* data)
{
	logInfo("syscall not implemented: syscallGetParentProcessId");
}

void syscallGetMilliseconds(g_task* task, g_syscall_millis* data)
{
	data->millis = clockGetLocal()->time;
}

void syscallGetExecutablePath(g_task* task, g_syscall_get_executable_path* data)
{
	if(task->process->environment.executablePath)
		stringCopy(data->buffer, task->process->environment.executablePath);
	else
		data->buffer[0] = 0;
}

void syscallGetWorkingDirectory(g_task* task, g_syscall_get_working_directory* data)
{
	const char* workingDirectory = task->process->environment.workingDirectory;
	if(workingDirectory)
	{
		size_t length = stringLength(workingDirectory);
		if(length + 1 > data->maxlen)
		{
			data->result = G_GET_WORKING_DIRECTORY_SIZE_EXCEEDED;
		}
		else
		{
			stringCopy(data->buffer, workingDirectory);
			data->result = G_GET_WORKING_DIRECTORY_SUCCESSFUL;
		}
	}
	else
	{
		stringCopy(data->buffer, "/");
		data->result = G_GET_WORKING_DIRECTORY_SUCCESSFUL;
	}
}

void syscallSetWorkingDirectory(g_task* task, g_syscall_set_working_directory* data)
{
	auto findRes = filesystemFind(nullptr, data->path);
	if(findRes.status == G_FS_OPEN_SUCCESSFUL)
	{
		if(findRes.file->type == G_FS_NODE_TYPE_FOLDER || findRes.file->type == G_FS_NODE_TYPE_MOUNTPOINT || findRes.
		   file->type == G_FS_NODE_TYPE_ROOT)
		{
			if(task->process->environment.workingDirectory)
			{
				heapFree(task->process->environment.workingDirectory);
			}

			int length = filesystemGetAbsolutePathLength(findRes.file);
			task->process->environment.workingDirectory = (char*) heapAllocate(length + 1);
			filesystemGetAbsolutePath(findRes.file, task->process->environment.workingDirectory);
			data->result = G_SET_WORKING_DIRECTORY_SUCCESSFUL;
		}
		else
		{
			data->result = G_SET_WORKING_DIRECTORY_NOT_A_FOLDER;
		}
	}
	else if(findRes.status == G_FS_OPEN_NOT_FOUND)
	{
		data->result = G_SET_WORKING_DIRECTORY_NOT_FOUND;
	}
	else
	{
		data->result = G_SET_WORKING_DIRECTORY_ERROR;
	}
}

void syscallReleaseCliArguments(g_task* task, g_syscall_cli_args_release* data)
{
	if(task->process->environment.arguments)
		stringCopy(data->buffer, task->process->environment.arguments);
	else
		data->buffer[0] = 0;
}

void syscallRegisterTaskIdentifier(g_task* task, g_syscall_task_id_register* data)
{
	data->successful = taskingDirectoryRegister(data->identifier, task->id, task->securityLevel);
}

void syscallGetTaskForIdentifier(g_task* task, g_syscall_task_id_get* data)
{
	data->resultTaskId = taskingDirectoryGet(data->identifier);
}
