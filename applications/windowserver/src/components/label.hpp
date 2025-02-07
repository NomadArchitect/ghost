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

#ifndef __WINDOWSERVER_COMPONENTS_LABEL__
#define __WINDOWSERVER_COMPONENTS_LABEL__

#include "components/component.hpp"
#include <libfont/font.hpp>
#include <libfont/text_alignment.hpp>
#include "components/titled_component.hpp"

class label_t : public component_t, public titled_component_t
{
private:
    g_font* font;
    int fontSize;
    cairo_text_extents_t lastExtents;

    std::string text;
    g_text_alignment alignment;
    g_color_argb color;

public:
    label_t();

    virtual ~label_t()
    {
    }

    void paint() override;
    void layout() override;
    void update() override;

    virtual void setFont(g_font* font);

    virtual void setColor(g_color_argb color);
    virtual g_color_argb getColor();

    void setTitle(std::string title) override;
    std::string getTitle() override;

    void setAlignment(g_text_alignment alignment);
    g_text_alignment getAlignment();

    bool setNumericProperty(int property, uint32_t value) override;
};

#endif
