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

#include "modes/text/66x25.hpp"

#include "modes/draw/draw_400_o.hpp"

#include "memory/video_ram.hpp"

namespace vga
{
namespace modes
{
namespace text
{

constexpr int font_height = 8;
constexpr int font_width = 5;

Mode66x25::Mode66x25(vga::Vga& vga)
    : text_buffer_(video_ram + static_cast<int>(
        get_height() * font_height * get_width() * font_width / sizeof(uint32_t) / 1.25
    ) + 1)
{
    vga.setup_draw_function(&draw_400_6bit_wrapper);
}

void Mode66x25::write(int column, int row, char c)
{

}

void Mode66x25::set_pixel(msgui::Position position, int color)
{
    const int pixel_slot = position.y * get_height() / sizeof(uint32_t) / 1.25; // 5 pixels in uint32_t
    constexpr int bits_per_pixel = 6;
    constexpr int pixels_in_4bytes = 5;
    const int offset = bits_per_pixel * (position.x % pixels_in_4bytes);
    video_ram[pixel_slot] &= ~(0x3f << offset);
    video_ram[pixel_slot] |= (color << offset);
}


} // namespace text
} // namespace modes
} // namespace vga



