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

#include "kernel/system/system.hpp"
#include "kernel/system/interrupts/interrupts.hpp"
#include "kernel/system/interrupts/lapic.hpp"
#include "kernel/system/interrupts/ioapic.hpp"
#include "kernel/system/interrupts/pic.hpp"
#include "kernel/system/acpi/acpi.hpp"
#include "kernel/memory/gdt.hpp"
#include "kernel/kernel.hpp"

static int applicationCoresWaiting;
static bool bspInitialized = false;

void systemInitializeBsp()
{
	processorInitializeBsp();

	acpiInitialize();

	if(!(lapicGlobalIsPrepared() && ioapicAreAvailable() && processorListAvailable()))
		kernelPanic("%! pic compatibility mode not implemented. apic/ioapic required!", "system");

	idtPrepare();
	idtLoad();

	picDisable();
	lapicInitialize();
	ioapicInitializeAll();

	gdtPrepare();
	gdtInitialize();

	applicationCoresWaiting = processorGetNumberOfCores() - 1;
	bspInitialized = true;
}

void systemInitializeAp()
{
	idtLoad();
	processorInitializeAp();
	lapicInitialize();

	gdtInitialize();
	systemMarkApplicationCoreReady();
}

void systemWaitForApplicationCores()
{
	logInfo("%! waiting for %i application processors", "kern", applicationCoresWaiting);
	while(applicationCoresWaiting > 0)
		asm("pause");
}

void systemMarkApplicationCoreReady()
{
	--applicationCoresWaiting;
}
