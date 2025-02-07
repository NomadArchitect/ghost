/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *  Ghost, a micro-kernel based operating system for the x86 architecture    *
 *  Copyright (C) 2022, Max Schlüssel <lokoxe@gmail.com>                     *
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

#include "windowserver.hpp"
#include "components/button.hpp"
#include "components/cursor.hpp"
#include "events/event.hpp"
#include "events/locatable.hpp"
#include "input/input_receiver.hpp"
#include "interface/interface_responder.hpp"
#include "interface/registration_thread.hpp"
#include "video/vbe_video_output.hpp"
#include "video/vmsvga_video_output.hpp"

#include <cairo/cairo.h>
#include <iostream>
#include <cstdio>
#include <test.hpp>
#include <components/window.hpp>
#include <components/text/text_field.hpp>
#include <layout/grid_layout_manager.hpp>

static windowserver_t* server;
static g_user_mutex dispatchLock = g_mutex_initialize_r(true);
static int framesTotal = 0;
static bool debugOn = false;

int main()
{
	server = new windowserver_t();
	server->launch();
	return 0;
}

windowserver_t* windowserver_t::instance()
{
	return server;
}

void windowserver_t::initializeInput()
{
	input_receiver_t::initialize();

	std::string keyLayout = "de-DE";
	if(!g_keyboard::loadLayout(keyLayout))
	{
		klog(("failed to load keyboard layout '" + keyLayout + "', no keyboard input available").c_str());
	}

	server->loadCursor();
	server->triggerRender();
}

void startOtherTasks()
{
	// TODO not the windowservers job
	g_spawn("/applications/desktop.bin", "", "", G_SECURITY_LEVEL_APPLICATION);
}

void windowserver_t::launch()
{
	eventProcessor = new event_processor_t();

	g_task_register_id("windowserver");
	initializeGraphics();
	g_create_task((void*) &windowserver_t::initializeInput);

	g_dimension resolution = videoOutput->getResolution();
	g_rectangle screenBounds(0, 0, resolution.width, resolution.height);
	createVitalComponents(screenBounds);

	startInterface();

	g_create_task((void*) &startOtherTasks);

	renderLoop(screenBounds);
}

void windowserver_t::createVitalComponents(g_rectangle screenBounds)
{
	screen = new screen_t();
	screen->setBounds(screenBounds);

	stateLabel = new label_t();
	stateLabel->setTitle("");
	stateLabel->setAlignment(g_text_alignment::RIGHT);
	stateLabel->setBounds(g_rectangle(10, screenBounds.height - 30, screenBounds.width - 20, 30));
	instance()->stateLabel->setColor(RGB(255, 255, 255));
	screen->addChild(stateLabel);
	g_create_task((void*) &windowserver_t::fpsCounter);
}

void windowserver_t::initializeGraphics()
{
	g_set_video_log(false);

	auto vmsvgaOutput = new g_vmsvga_video_output();
	if(vmsvgaOutput->initialize())
	{
		videoOutput = vmsvgaOutput;
	}
	else
	{
		klog("failed to initialize VMSVGA video output, trying VESA");

		auto vbeOutput = new g_vbe_video_output();
		if(vbeOutput->initialize())
		{
			videoOutput = vbeOutput;
		}
		else
		{
			klog("failed to initialize VBE output, exiting");
			g_exit(0);
		}
	}
}

void windowserver_t::renderLoop(g_rectangle screenBounds)
{
	g_task_register_id("windowserver/renderer");

	graphics_t global;
	global.resize(screenBounds.width, screenBounds.height, false);

	cursor_t::nextPosition = g_point(screenBounds.width / 2, screenBounds.height / 2);

	g_mutex_acquire(renderAtom);
	while(true)
	{
		eventProcessor->process();

		screen->resolveRequirement(COMPONENT_REQUIREMENT_UPDATE, 0);
		screen->resolveRequirement(COMPONENT_REQUIREMENT_LAYOUT, 0);
		screen->resolveRequirement(COMPONENT_REQUIREMENT_PAINT, 0);

		screen->blit(&global, screenBounds, g_point(0, 0));
		cursor_t::paint(&global);

		output(&global);

		framesTotal++;
		g_mutex_acquire_to(renderAtom, 1000);
	}
}

void windowserver_t::triggerRender()
{
	g_mutex_release(renderAtom);
}

void windowserver_t::output(graphics_t* graphics)
{
	g_rectangle invalid = screen->grabInvalid();
	if(invalid.width == 0 && invalid.height == 0)
		return;

	g_dimension resolution = videoOutput->getResolution();
	g_rectangle screenBounds(0, 0, resolution.width, resolution.height);

	auto screenSurface = graphics->getSurface();
	cairo_surface_flush(screenSurface);
	g_color_argb* buffer = (g_color_argb*) cairo_image_surface_get_data(screenSurface);
	videoOutput->blit(invalid, screenBounds, buffer);
}

void windowserver_t::loadCursor()
{
	cursor_t::load("/system/graphics/cursor/default.cursor");
	cursor_t::load("/system/graphics/cursor/text.cursor");
	cursor_t::load("/system/graphics/cursor/resize-ns.cursor");
	cursor_t::load("/system/graphics/cursor/resize-ew.cursor");
	cursor_t::load("/system/graphics/cursor/resize-nesw.cursor");
	cursor_t::load("/system/graphics/cursor/resize-nwes.cursor");
	cursor_t::set("default");
}

component_t* windowserver_t::dispatchUpwards(component_t* component, event_t& event)
{
	// store when dispatching to parents
	g_point initialPosition;
	locatable_t* locatable = dynamic_cast<locatable_t*>(&event);
	if(locatable)
	{
		initialPosition = locatable->position;
	}

	// check upwards until someone accepts the event
	component_t* acceptor = component;
	while(dispatch(acceptor, event) == nullptr)
	{
		acceptor = acceptor->getParent();
		if(acceptor == 0)
		{
			break;
		}

		// restore position on locatable events
		if(locatable)
		{
			locatable->position = initialPosition;
		}
	}
	return acceptor;
}

component_t* windowserver_t::dispatch(component_t* component, event_t& event)
{
	component_t* handledBy = nullptr;

	if(component->canHandleEvents())
	{
		locatable_t* locatable = dynamic_cast<locatable_t*>(&event);
		if(locatable != 0)
		{
			g_point locationOnScreen = component->getLocationOnScreen();
			locatable->position.x -= locationOnScreen.x;
			locatable->position.y -= locationOnScreen.y;
		}

		g_mutex_acquire(dispatchLock);
		handledBy = event.visit(component);
		g_mutex_release(dispatchLock);
	}

	return handledBy;
}

void windowserver_t::fpsCounter()
{
	g_task_register_id("windowserver/fps-counter");

	int tenthSeconds = 0;
	for(;;)
	{
		g_sleep(100);
		tenthSeconds++;
		std::stringstream s;
		s
				<< "FPS: " << framesTotal << ", "
				<< "TIME: "
				<< "System: " << ((double) g_millis()) / 1000.0 << "s, "
				<< "Application: " << tenthSeconds / 10;
		instance()->stateLabel->setTitle(s.str());
		instance()->triggerRender();
		framesTotal = 0;
	}
}

void windowserver_t::startInterface()
{
	g_create_task((void*) &interfaceRegistrationThread);
	g_create_task((void*) &interfaceResponderThread);
}

void windowserver_t::setDebug(bool cond)
{
	debugOn = cond;
}

bool windowserver_t::isDebug()
{
	return debugOn;
}
