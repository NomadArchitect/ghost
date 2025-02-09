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

#include "libwindow/component_registry.hpp"
#include "libwindow/component.hpp"
#include <map>

static g_user_mutex componentsLock = g_mutex_initialize();
static std::map<g_ui_component_id, g_component*> components;

/**
 *
 */
void g_component_registry::add(g_component* component)
{
	g_mutex_acquire(componentsLock);
	components[component->getId()] = component;
	g_mutex_release(componentsLock);
}

/**
 *
 */
g_component* g_component_registry::get(g_ui_component_id id)
{
	g_mutex_acquire(componentsLock);

	g_component* component = nullptr;
	if(components.count(id) > 0)
		component = components[id];

	g_mutex_release(componentsLock);

	return component;
}
