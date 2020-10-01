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

namespace vga
{
namespace modes
{
namespace text
{

class Mode66x25
{
public:
    Mode66x25(Vga& vga);

    constexpr static int get_height()
    {
        return 25;
    }

    constexpr static int get_width()
    {
        return 66;
    }

    void write(int column, int row, char c);
    void write(const char c);


private:
    void set_pixel(msgui::Position position, int color);

    uint32_t *text_buffer_;

    msgui::Position cursor_{0, 0};
    vga::Vga vga_;
};

} // namespace text
} // namespace modes
} // namespace vga



