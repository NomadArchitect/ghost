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

#include "desktop.hpp"
#include "background.hpp"
#include "taskbar.hpp"

#include <fstream>
#include <libproperties/parser.hpp>

#define USER_DESKTOP_DIR "/user/desktop/"

background_t* background = nullptr;
taskbar_t* taskbar = nullptr;

int main()
{
	if(g_ui::open() != G_UI_OPEN_STATUS_SUCCESSFUL)
	{
		klog("failed to create UI");
		return -1;
	}

	background = background_t::create();
	g_ui::registerDesktopCanvas(background);

	g_dimension screenDimension;
	g_ui::getScreenDimension(screenDimension);
	taskbar = taskbar_t::create();
	taskbar->setBounds(g_rectangle(0, screenDimension.height - 40, screenDimension.width, 40));
	background->addTaskbar(taskbar);
	background->setDesktopListener([](g_ui_windows_event* event)
	{
		taskbar->handleDesktopEvent(event);
	});

	desktopLoadItems();

	for(;;)
	{
		g_sleep(999999);
	}
}

void desktopLoadItems()
{
	auto dir = g_open_directory(USER_DESKTOP_DIR);

	int next = 0;
	g_fs_directory_entry* entry;
	while((entry = g_read_directory(dir)) != nullptr)
	{
		std::string path = std::string(USER_DESKTOP_DIR) + entry->name;

		std::ifstream cfg(path);
		g_properties_parser parser(cfg);
		auto properties = parser.getProperties();

		auto item = item_t::create(properties["name"], properties["icon"], properties["application"]);
		background->addItem(item);
		item->setBounds(g_rectangle(0, next++ * 100, 100, 100));
	}
	background->organize();
}
