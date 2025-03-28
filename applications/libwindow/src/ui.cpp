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

#include <ghost.h>
#include <stdio.h>

#include "libwindow/canvas.hpp"
#include "libwindow/component.hpp"
#include "libwindow/ui.hpp"

/**
 * Global ready indicator
 */
bool g_ui_initialized = false;

/**
 * ID of the window server interface receiver task
 */
g_tid g_ui_delegate_tid = -1;

/**
 * Our local event dispatcher task ID
 */
g_tid g_ui_event_dispatcher_tid = -1;

/**
 * Opens a connection to the window server.
 */
g_ui_open_status g_ui::open()
{
	// check if already open
	if(g_ui_initialized)
	{
		return G_UI_OPEN_STATUS_EXISTING;
	}

	// get window managers id
	g_tid windowserverRegistryTask = g_task_await_by_name(G_UI_REGISTRY_NAME);
	if(windowserverRegistryTask == G_TID_NONE)
	{
		klog("failed to retrieve task id of window server with identifier '%s'", G_UI_REGISTRY_NAME);
		return G_UI_OPEN_STATUS_COMMUNICATION_FAILED;
	}

	// start event dispatcher
	g_ui_event_dispatcher_tid = g_create_task((void*) &eventDispatchThread);

	// send initialization request
	g_message_transaction init_tx = g_get_message_tx_id();

	g_ui_initialize_request request;
	request.header.id = G_UI_PROTOCOL_INITIALIZATION;
	request.event_dispatcher = g_ui_event_dispatcher_tid;
	g_send_message_t(windowserverRegistryTask, &request, sizeof(g_ui_initialize_request), init_tx);
	g_yield_t(g_ui_delegate_tid);

	// receive initialization response
	size_t buflen = sizeof(g_message_header) + sizeof(g_ui_initialize_response);
	uint8_t* buf[buflen];
	if(g_receive_message_t(buf, buflen, init_tx) != G_MESSAGE_RECEIVE_STATUS_SUCCESSFUL)
	{
		klog("failed to communicate with the window server");
		return G_UI_OPEN_STATUS_COMMUNICATION_FAILED;
	}

	// check response
	auto response = (g_ui_initialize_response*) G_MESSAGE_CONTENT(buf);
	if(response->status != G_UI_PROTOCOL_SUCCESS)
	{
		klog("failed to open UI");
		return G_UI_OPEN_STATUS_FAILED;
	}

	// mark UI as ready
	g_ui_initialized = true;
	g_ui_delegate_tid = response->window_server_delegate;
	return G_UI_OPEN_STATUS_SUCCESSFUL;
}

/**
 *
 */
bool g_ui::addListener(g_ui_component_id id, g_ui_component_event_type eventType)
{
	g_message_transaction tx = g_get_message_tx_id();

	g_ui_component_add_listener_request request;
	request.header.id = G_UI_PROTOCOL_ADD_LISTENER;
	request.id = id;
	request.target_thread = g_ui_event_dispatcher_tid;
	request.event_type = eventType;
	g_send_message_t(g_ui_delegate_tid, &request, sizeof(g_ui_component_add_listener_request), tx);
	g_yield_t(g_ui_delegate_tid);

	size_t bufferSize = sizeof(g_message_header) + sizeof(g_ui_component_add_listener_response);
	uint8_t buffer[bufferSize];
	if(g_receive_message_t(buffer, bufferSize, tx) == G_MESSAGE_RECEIVE_STATUS_SUCCESSFUL)
	{
		auto response = (g_ui_component_add_listener_response*) G_MESSAGE_CONTENT(buffer);
		return response->status == G_UI_PROTOCOL_SUCCESS;
	}
	return false;
}

/**
 *
 */
void g_ui::eventDispatchThread()
{
	size_t bufLen = G_UI_MAXIMUM_MESSAGE_SIZE;
	auto buf = new uint8_t[bufLen];

	while(true)
	{
		auto stat = g_receive_message(buf, bufLen);
		if(stat == G_MESSAGE_RECEIVE_STATUS_SUCCESSFUL)
		{
			// event message
			auto event_header = (g_ui_component_event_header*) G_MESSAGE_CONTENT(buf);
			g_component* component = g_component_registry::get(event_header->component_id);

			if(component == nullptr)
			{
				klog("event received for unknown component %i", event_header->component_id);
				continue;
			}

			component->handle(event_header);
		}
		else
		{
			klog("something went wrong when receiving an event, status code: %i", stat);
		}
	}

	delete buf;
}

/**
 *
 */
bool g_ui::registerDesktopCanvas(g_canvas* c)
{
	if(!g_ui_initialized)
	{
		return false;
	}

	g_message_transaction tx = g_get_message_tx_id();

	// send registration request
	g_ui_register_desktop_canvas_request request;
	request.header.id = G_UI_PROTOCOL_REGISTER_DESKTOP_CANVAS;
	request.canvas_id = c->getId();
	request.target_thread = g_ui_event_dispatcher_tid;
	g_send_message_t(g_ui_delegate_tid, &request, sizeof(g_ui_register_desktop_canvas_request), tx);
	g_yield_t(g_ui_delegate_tid);

	// read response
	size_t buflen = sizeof(g_message_header) + sizeof(g_ui_register_desktop_canvas_response);
	uint8_t buf[buflen];

	bool success = false;
	if(g_receive_message_t(buf, buflen, tx) == G_MESSAGE_RECEIVE_STATUS_SUCCESSFUL)
	{
		auto response = (g_ui_register_desktop_canvas_response*) G_MESSAGE_CONTENT(buf);
		success = (response->status == G_UI_PROTOCOL_SUCCESS);
	}
	return success;
}

/**
 *
 */
bool g_ui::getScreenDimension(g_dimension& out)
{
	if(!g_ui_initialized)
	{
		return false;
	}

	g_message_transaction tx = g_get_message_tx_id();

	// send request
	g_ui_get_screen_dimension_request request;
	request.header.id = G_UI_PROTOCOL_GET_SCREEN_DIMENSION;
	g_send_message_t(g_ui_delegate_tid, &request, sizeof(g_ui_get_screen_dimension_request), tx);
	g_yield_t(g_ui_delegate_tid);

	// read response
	size_t bufferSize = sizeof(g_message_header) + sizeof(g_ui_get_screen_dimension_response);
	uint8_t* buffer = new uint8_t[bufferSize];

	bool success = false;
	if(g_receive_message_t(buffer, bufferSize, tx) == G_MESSAGE_RECEIVE_STATUS_SUCCESSFUL)
	{
		g_ui_get_screen_dimension_response* response = (g_ui_get_screen_dimension_response*) G_MESSAGE_CONTENT(buffer);
		out = response->size;
		success = true;
	}

	return success;
}
