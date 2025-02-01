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

#ifndef __TERMINAL_SCREEN_GUISCREEN__
#define __TERMINAL_SCREEN_GUISCREEN__

#include "../screen.hpp"
#include "terminal_document.hpp"
#include "char_renderer.hpp"

#include <cairo/cairo.h>
#include <list>
#include <map>

#include <libwindow/button.hpp>
#include <libwindow/canvas.hpp>
#include <libwindow/listener/focus_listener.hpp>
#include <libwindow/listener/key_listener.hpp>
#include <libwindow/ui.hpp>
#include <libwindow/window.hpp>

class canvas_resize_bounds_listener_t;
class input_key_listener_t;
class canvas_buffer_listener_t;
class terminal_focus_listener_t;

class gui_screen_t : public screen_t
{
    g_user_mutex exitFlag;
    g_window* window;

    std::list<g_key_info> inputBuffer;
    g_user_mutex inputBufferEmpty;
    g_user_mutex inputBufferLock;

    bool fullRepaint;
    g_user_mutex upToDate = g_mutex_initialize();
    bool focused = false;
    uint64_t lastInputTime = 0;
    int scrollY = 0;

    terminal_document_t* document = new terminal_document_t();
    int documentRows = 0;

    struct
    {
        bool blink = false;
        int x = 0;
        int y = 0;
        int width = 1;
    } cursor;

    struct
    {
        g_canvas* component;
        g_rectangle bounds;

        cairo_surface_t* surface = nullptr;
        uint8_t* surfaceBuffer = nullptr;
        g_dimension bufferSize;
        cairo_t* context = nullptr;

        int padding = 3;
    } canvas;

    struct
    {
        int width = 8;
        int height = 18;
        double fontSize = 14;
        char_renderer_t* renderer = new char_renderer_t();
    } chars;

    struct
    {
        g_size max{};
        char* buffer = nullptr;
        int columns = 0;
        int rows = 0;
    } viewBuffer;

    void setCanvasBounds(g_rectangle& bounds);
    cairo_t* getGraphics();
    void repaint() const;
    void setFocused(bool focused);
    void bufferInput(const g_key_info& info);

    friend class canvas_resize_bounds_listener_t;
    friend class input_key_listener_t;
    friend class canvas_buffer_listener_t;
    friend class terminal_focus_listener_t;

public:
    bool createUi();
    void blinkCursor();
    void paint();
    void recalculateView();
    void normalizeScroll();

    bool initialize(g_user_mutex exitFlag) override;
    g_key_info readInput() override;
    void clean() override;
    void backspace() override;
    void remove() override;
    void write(char c) override;
    void flush() override;

    void setCursor(int x, int y) override;
    int getCursorX() override;
    int getCursorY() override;
    void setCursorVisible(bool visible) override;
    void setScrollAreaScreen() override;
    void setScrollArea(int start, int end) override;
    void scroll(int value) override;

    int getColumns() override;
    int getRows() override;
};

class canvas_resize_bounds_listener_t : public g_bounds_listener
{
    gui_screen_t* screen;

public:
    explicit canvas_resize_bounds_listener_t(gui_screen_t* screen) : screen(screen)
    {
    }

    void handle_bounds_changed(g_rectangle bounds) override;
};

class input_key_listener_t : public g_key_listener
{
    gui_screen_t* screen;

public:
    explicit input_key_listener_t(gui_screen_t* screen) : screen(screen)
    {
    }

    void handle_key_event(g_key_event& e) override;
};

class canvas_buffer_listener_t : public g_canvas_buffer_listener
{
    gui_screen_t* screen;

public:
    explicit canvas_buffer_listener_t(gui_screen_t* screen) : screen(screen)
    {
    }

    void handle_buffer_changed() override;
};

class terminal_focus_listener_t : public g_focus_listener
{
private:
    gui_screen_t* screen;

public:
    terminal_focus_listener_t(gui_screen_t* screen) : screen(screen)
    {
    }

    virtual void handle_focus_changed(bool now_focused);
};

#endif
