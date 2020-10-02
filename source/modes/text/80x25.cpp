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

#include "modes/text/80x25.hpp"

#include <cstring>

#include "modes/draw/draw_480_6bit.hpp"

#include "memory/video_ram.hpp"

#include <msgui/fonts/Font5x7.hpp>

#include "interfaces/usart.hpp"


namespace vga
{
namespace modes
{
namespace text
{

namespace
{
constexpr int font_height = 8;
constexpr int font_width = 6;

constexpr int width_pixels = 480;
constexpr int height_pixels = 200;

constexpr auto font = msgui::fonts::Font5x7::data;

constexpr int video_ram_size = width_pixels * height_pixels * 0.8 + 4;
constexpr int text_buffer_size = 80 * 25;
constexpr int attributes_size = 80 * 25;
constexpr int delta_size = 80 * 25 / 8;

}

constexpr uint32_t get_mask(const auto& bitmap, const int y, const int x)
{
    return bitmap.getPixel(x, y) == 1 ? 0x1F : 0x00;
}

void Mode80x25::render_font(const auto& bitmap, const int row, const int column, const int color)
{
    for (int y = 0; y < 7; ++y)
    {
        for (int x = 0; x < 5; ++x)
        {
            if (bitmap.getPixel(x, y))
            {
                set_pixel({.x = column + x, .y = row + y}, color);
            }
            else
            {
                set_pixel({.x = column + x, .y = row + y}, 0);
            }
        }
        set_pixel({.x = column + 5, .y = row + y}, 0);
    }
    for (int x = 0; x < 6; ++x)
    {
        set_pixel({.x = column + x, .y = row + 7}, 0);
    }
}

Mode80x25::Mode80x25(vga::Vga& vga)
    : text_buffer_(reinterpret_cast<char*>(
        reinterpret_cast<uint8_t*>(video_ram) + video_ram_size))
    , changed_bitmap_(reinterpret_cast<uint8_t*>(video_ram) + (video_ram_size + text_buffer_size))
{
    std::memset(changed_bitmap_, 0, delta_size);
    Vga::Config config {
        .draw = &draw_480_6bit_wrapper,
        .number_of_lines = get_height() * font_height,
        .lines_to_be_omitted = 1,
        .line_memory_offset = width_pixels / 5,
        .delay_for_line = 70,
        .line_multiplier = 2
    };
    vga.setup(config);
    // set_pixel({.x = 0, .y = 0}, 15);
    // set_pixel({.x = 10, .y = 10}, 15);

    // for (int i = 0; i < 400; ++i)
    // {
    //     set_pixel({.x = i, .y = 0}, 15);
    // }
    // for (int i = 0; i < get_width(); ++i)
    // // {
    // //     set_pixel({.x = i, .y = 0}, 15);
    // // }
    // video_ram[0] = 0x0f;
    render_test_box();

    std::memset(text_buffer_, 0, get_width() * get_height());
    // for (int y = 0; y < get_height(); ++y)
    // {
    //     for (int x = 0; x < get_width(); ++x)
    //     {
    //         const char c = text_buffer_[y * get_width() + x];
    //         if (c != 0)
    //         {
    //             render_font(font.get(c), y * font_height, x * font_width, 10);
    //         }
    //     }
    // }

}

void Mode80x25::render_test_box()
{
    for (int i = 0; i < width_pixels; ++i)
    {
        set_pixel({.x = i, .y = 0}, 15);
        set_pixel({.x = 0, .y = i % height_pixels}, 20);
        set_pixel({.x = width_pixels - 1, .y = i % height_pixels}, 17);
        set_pixel({.x = i, .y = height_pixels - 1}, 18);
    }
}


void Mode80x25::write(uint8_t row, uint8_t column, char c)
{
    if (column >= get_width() || row >= get_height())
    {
        return;
    }

    cursor_column_ = column + 1;
    cursor_row_ = row;

    if (cursor_column_ >= get_width())
    {
        cursor_column_ = 0;
        ++cursor_row_;
    }
    if (cursor_row_ >= get_height())
    {
        cursor_row_ = 0;
    }

    int y = row;
    int x = column;

    text_buffer_[y * get_width() + x] = c;

    const int char_position = row * 10 + column / 8;
    const int offset = column % 8;
    changed_bitmap_[char_position] &= ~(0x1 << offset);
    changed_bitmap_[char_position] |= 0x1 << offset;
}

void Mode80x25::write(char c)
{
    write(cursor_row_, cursor_column_, c);
}

bool one = true;
int counter = 0;
void Mode80x25::render()
{

    if (++counter > 30)
    {
        counter = 0;
        int pixel = 0;
        if (one)
        {
            pixel = 63;
            one = false;
        }
        else
        {
            one = true;
        }

        for (int y = 0; y < font_height; y++)
        {
            for (int x = 0; x < font_width; x++)
            {
                set_pixel({.x = cursor_column_ * font_width + x, .y = cursor_row_ * font_height + y}, pixel);
            }
        }
    }


    // render screen

    for (int y = 0; y < get_height(); ++y)
    {
        for (int x = 0; x < 10; ++x)
        {
            const int char_position = y * 10 + x;


            if (changed_bitmap_[char_position])
            {
                for (int i = 0; i < 8; ++i)
                {
                    if (changed_bitmap_[char_position] & (1 << i))
                    {
                        const char c = text_buffer_[y * 80 + (8 * x) + i];
                        render_font(font.get(c), y * font_height, x * 8 * font_width + i * font_width, 63);
                    }
                }
            }
        }
    }

    std::memset(changed_bitmap_, 0, delta_size);

    // render cursor



}

void Mode80x25::set_pixel(msgui::Position position, int color)
{
    const int pixel_slot = position.y * width_pixels / 5 + position.x / 5; // 5 pixels in uint32_t
    constexpr int bits_per_pixel = 6;
    constexpr int pixels_in_4bytes = 5;
    const int offset = bits_per_pixel * (position.x % pixels_in_4bytes);
    video_ram[pixel_slot] &= ~(0x3f << offset);
    video_ram[pixel_slot] |= (color << offset);
}

int Mode80x25::get_pixel(msgui::Position position)
{
    const int pixel_slot = position.y * width_pixels / 5 + position.x / 5; // 5 pixels in uint32_t
    constexpr int bits_per_pixel = 6;
    constexpr int pixels_in_4bytes = 5;
    const int offset = bits_per_pixel * (position.x % pixels_in_4bytes);
    return video_ram[pixel_slot] &= (0x3f << offset) >> offset;
}


} // namespace text
} // namespace modes
} // namespace vga



