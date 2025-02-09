/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *  Ghost, a micro-kernel based operating system for the x86 architecture    *
 *  Copyright (C) 2025, Max Schlüssel <lokoxe@gmail.com>                     *
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

#ifndef LIBWINDOW_MOUSELISTENER
#define LIBWINDOW_MOUSELISTENER

#include "libwindow/listener/listener.hpp"
#include <functional>
#include <utility>
#include <bits/std_function.h>

typedef std::function<void(g_ui_component_mouse_event*)> g_mouse_listener_func;

class g_mouse_listener : public g_listener
{
public:
    ~g_mouse_listener() override = default;

    void process(g_ui_component_event_header* header) override
    {
        handleMouseEvent((g_ui_component_mouse_event*)header);
    }

    virtual void handleMouseEvent(g_ui_component_mouse_event* e) = 0;
};

class g_mouse_listener_dispatcher : public g_mouse_listener
{
    g_mouse_listener_func func;

public:
    explicit g_mouse_listener_dispatcher(g_mouse_listener_func func) : func(std::move(func))
    {
    };

    void handleMouseEvent(g_ui_component_mouse_event* e) override
    {
        func(e);
    }
};

#endif
