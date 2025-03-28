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

#ifndef __KERNEL_TASKING__
#define __KERNEL_TASKING__

#include "kernel/tasking/task.hpp"
#include "kernel/utils/hashmap.hpp"
#include "kernel/runtime/itanium_cxx_abi_support.hpp"

#include <ghost/system/types.h>
#include <ghost/tasks/types.h>

extern g_hashmap<g_tid, g_task*>* taskGlobalMap;

struct g_schedule_entry
{
    g_task* task;
    g_schedule_entry* next;
};

/**
 * Processor local tasking structure. For each processor there is one instance
 * of this struct that contains the current state.
 */
struct g_tasking_local
{
    g_mutex lock;
    uint32_t processor;

    struct
    {
        /**
         * Each first acquire of a global mutex increases the count on this
         * processor, each release decreases it.
         */
        int globalLockCount;

        /**
         * As long as a global mutex is held, interrupts are disabled and this
         * flag holds the IF state on the first acquire. On the last release,
         * this state is restored.
         */
        bool globalLockSetIFAfterRelease;
    } locking;

    /**
     * Scheduling information for this processor.
     */
    struct
    {
        g_schedule_entry* list;
        g_task* current;

        g_task* idleTask;
    } scheduling;
};

struct g_spawn_result
{
    g_spawn_status status;
    g_process* process;
    g_spawn_validation_details validation;
};

/**
 * @return the processor-local tasking structure
 */
g_tasking_local* taskingGetLocal();

/**
 * @return the task that is on this processor currently running or was
 * last running when called from within a system call handler
 */
g_task* taskingGetCurrentTask();

/**
 * @return the next assignable task id
 */
g_tid taskingGetNextId();

/**
 * Initializes basic task management and necessary structures.
 */
void taskingInitializeBsp();

/**
 * Initializes the local task management for this core.
 */
void taskingInitializeAp();

/**
 * Initializes the processor-local tasking structure.
 */
void taskingInitializeLocal();

/**
 *
 */
void taskingCreateEssentialTasks();

/**
 * Adds a task to the list of scheduled tasks on the given local.
 */
void taskingAssign(g_tasking_local* local, g_task* task);

/**
 * Assign a task to any core with the least load.
 */
void taskingAssignBalanced(g_task* task);

/**
 * Assigns a task to a specific core.
 */
void taskingAssignOnCore(uint8_t core, g_task* task);

/**
 * Creates an empty process. Creates a new page directory with the kernel areas
 * correctly mapped and instantiates a virtual range allocator.
 *
 * Doesn't create a thread.
 *
 * @param securityLevel
 *   security level to apply for the process
 */
g_process* taskingCreateProcess(g_security_level securityLevel);

/**
 * Creates a task that starts execution on the given entry. The task is added to the
 * task list of the specified process. The task is scheduled only after using <taskingAssign>.
 *
 * @param entry
 * 		execution entry of the thread
 * @param process
 * 		parent process
 * @param level
 * 		security level to apply for the thread
 * @return the task or null
 */
g_task* taskingCreateTask(g_virtual_address entry, g_process* process, g_security_level level);

/**
 * Creates a special kind of task that performs a virtual 8086 call.
 */
g_task* taskingCreateTaskVm86(g_process* process, uint32_t intr, g_vm86_registers in, g_vm86_registers* out);

/**
 * Removes a thread. Cleaning up all allocated data where possible.
 */
void taskingDestroyTask(g_task* task);

/**
 * Removes a process. Cleaning up all allocated data where possible.
 */
void taskingDestroyProcess(g_process* process);

/**
 * Kills all tasks of a process.
 */
void taskingProcessKillAllTasks(g_pid pid);

/**
 * Adds the task to the process task list.
 */
void taskingProcessAddToTaskList(g_process* process, g_task* task);

/**
 * Removes the task from its process task list.
 */
void taskingProcessRemoveFromTaskList(g_task* task);

/**
 * Schedules and sets the next task as the current. May only be called during interrupt handling!
 */
void taskingSchedule(bool resetPreference = false);

/**
 * Immediately sets a given task as the next one to execute. Dangerous because this only
 * works on the core that this task actually runs on right now.
 */
void taskingSetCurrent(g_task* task);

/**
 * Saves the state pointer that points to the stored state on the tasks kernel
 * stack. Also stores additional registers like for SSE.
 */
void taskingSaveState(g_task* task, g_processor_state* state);

/**
 * Applies the context switch to the task which is the current one for this core. This sets
 * the correct page directory and TLS variables.
 */
void taskingRestoreState(g_task* task);

/**
 * Yields control to the next task. This can only be called while no mutexes
 * are currently acquired by this thread, otherwise the kernel could get deadlocked.
 */
void taskingYield();

/**
 * Exits the current task. Sets the status to dead and yields.
 */
void taskingExit();

/**
 * Kernel thread, used by the scheduler when it should idle.
 */
void taskingIdleThread();

/**
 * Finds a task by its id.
 */
g_task* taskingGetById(g_tid id);

/**
 * Spawns an executable. This creates a task entering <taskingSpawnEntry> where the actual
 * binary loading happens. Makes the executing task wait for the spawn to complete.
 */
g_spawn_result taskingSpawn(g_fd fd, g_security_level securityLevel);

/**
 * Entry function for a newly spawned task. Loads the executable binary and then executes a
 * privilege downgrade to enter the user-level task execution.
 */
void taskingSpawnEntry();

/**
 * This function is entered after the binary is loaded within the kernel thread and the task
 * should escape to userland. Finally sets the task waiting until the caller task wakes it.
 */
void taskingFinalizeSpawn(g_task* task);

/**
 * Waits until the task exits and then wakes the waiting task.
 */
void taskingWaitForExit(g_tid task, g_tid waiter);

/**
 * Wakes the task.
 */
void taskingWake(g_task* task);

/**
 * Sets the task waiting and executes the function before yielding. The lambda
 * should attach wake-up for the task.
 */
void taskingWait(g_task* task, const char* debugName, const std::function<void ()>& beforeYield = nullptr);

#endif
