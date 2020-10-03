// This file is part of MS GPU project.
// Copyright (C) 2020 Mateusz Stadnik
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <msgui/fonts/Font5x7.hpp>

#include <msgui/Position.hpp>

#include "generator/vga.hpp"

#include "modes/types.hpp"

namespace vga
{
namespace modes
{
namespace text
{

class Mode80x25
{
public:
    using type = Text;

    Mode80x25(Vga& vga);

    constexpr static int get_height()
    {
        return 25;
    }

    constexpr static int get_width()
    {
        return 80;
    }

    void write(uint8_t row, uint8_t column, char c);
    void write(const char c);

    void render_test_box();
    void render();
    int get_pixel(msgui::Position position);

    void move_cursor(int row_offset, int column_offset);

    void set_cursor_row(int row);
    void set_cursor_column(int column);
    void set_cursor(int row, int column);
    void set_foreground_color(int foreground);
    void set_background_color(int background);
    void set_color(int foreground, int background);
private:
    void set_pixel(msgui::Position position, int color);
    void render_font(const auto& bitmap, const int row, const int column, const int foreground, const int background);

    char *text_buffer_;
    uint8_t* changed_bitmap_;
    uint16_t* attributes_;

    int foreground_ = 63;
    int background_ = 0;

    uint8_t cursor_row_{0};
    uint8_t cursor_column_{0};

    vga::Vga vga_;
    bool force_trigger_ {false};
};

} // namespace text
} // namespace modes
} // namespace vga



