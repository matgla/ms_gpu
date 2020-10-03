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
    // 76800 frame buffer
    // 2000 text buffer
    // 250 delta
    // 4000 attributes

template <typename Font>
struct Configuration
{
    constexpr static auto font = Font::data;

    constexpr static int character_height = font::height + 1;
    constexpr static int character_width = font::width + 1;

    constexpr static int resolution_width = 480;
    constexpr static int resolution_height = 200;

    constexpr static int video_ram_size = resolution_width * resolution_height * 0.8;
    constexpr static int text_buffer_size = 80 * 25;
    constexpr static int attributes_size = 80 * 25 * 2;
    constexpr static int delta_bitmap_size = 80 * 25 / 8;
};

constexpr static Configuration<msgui::fonts::Font5x7> configuration;

}

constexpr uint32_t get_mask(const auto& bitmap, const int y, const int x)
{
    return bitmap.getPixel(x, y) == 1 ? 0x1F : 0x00;
}

void Mode80x25::render_font(const auto& bitmap, const int row, const int column, const int foreground, const int background)
{
    for (int y = 0; y < 7; ++y)
    {
        for (int x = 0; x < 5; ++x)
        {
            if (bitmap.getPixel(x, y))
            {
                set_pixel({.x = column + x, .y = row + y}, foreground);
            }
            else
            {
                set_pixel({.x = column + x, .y = row + y}, background);
            }
        }
        set_pixel({.x = column + 5, .y = row + y}, background);
    }
    for (int x = 0; x < 6; ++x)
    {
        set_pixel({.x = column + x, .y = row + 7}, background);
    }
}

Mode80x25::Mode80x25(vga::Vga& vga)
    : text_buffer_(reinterpret_cast<char*>(
        reinterpret_cast<uint8_t*>(video_ram) + video_ram_size))
    , changed_bitmap_(reinterpret_cast<uint8_t*>(video_ram) + (video_ram_size + text_buffer_size))
    , attributes_(reinterpret_cast<uint16_t*>(changed_bitmap_ + delta_bitmap_size))
{
    std::memset(changed_bitmap_, 0, delta_bitmap_size);
    Vga::Config config {
        .draw = &draw_480_6bit_wrapper,
        .number_of_lines = get_height() * character_height,
        .lines_to_be_omitted = 1,
        .line_memory_offset = resolution_width / 5,
        .delay_for_line = 70,
        .line_multiplier = 2
    };
    vga.setup(config);
    render_test_box();

    std::memset(text_buffer_, 0, get_width() * get_height());
}

void Mode80x25::move_cursor(int row_offset, int column_offset)
{
    if (row_offset + cursor_row_ >= get_height() || row_offset + cursor_row_ < 0)
    {
        return;
    }

    if (column_offset + cursor_column_ >= get_width() || column_offset + cursor_column_ < 0)
    {
        return;
    }

    const int char_position = cursor_row_ * 10 + cursor_column_ / 8;
    const int offset = cursor_column_ % 8;
    changed_bitmap_[char_position] &= ~(0x1 << offset);
    changed_bitmap_[char_position] |= 0x1 << offset;

    cursor_column_ += column_offset;
    cursor_row_ += row_offset;
    force_trigger_ = true;
}

void Mode80x25::set_cursor_row(int row)
{
    if (row >= 0 && row <= get_height())
    {
        cursor_row_ = row;
    }
}

void Mode80x25::set_cursor_column(int column)
{
    if (column >= 0 && column <= get_width())
    {
        cursor_column_ = column;
    }
}

void Mode80x25::set_cursor(int row, int column)
{
    set_cursor_row(row);
    set_cursor_column(column);
}


void Mode80x25::render_test_box()
{
    for (int i = 0; i < resolution_width; ++i)
    {
        set_pixel({.x = i, .y = 0}, 15);
        set_pixel({.x = 0, .y = i % resolution_height}, 20);
        set_pixel({.x = resolution_width - 1, .y = i % resolution_height}, 17);
        set_pixel({.x = i, .y = resolution_height - 1}, 18);
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
    force_trigger_ = true;

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

    const int attribute_position = row * get_width() + column;
    attributes_[attribute_position] = foreground_ | (background_ << 6);
}

void Mode80x25::write(char c)
{
    write(cursor_row_, cursor_column_, c);
}

void Mode80x25::set_foreground_color(int foreground)
{
    foreground_ = foreground;
}

void Mode80x25::set_background_color(int background)
{
    background_ = background;
}

void Mode80x25::set_color(int foreground, int background)
{
    set_foreground_color(foreground);
    set_background_color(background);
}


bool one = true;
int counter = 0;
void Mode80x25::render()
{
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

                        const int attribute_position = y * get_width() + (x * 8 + i);
                        const int foreground = attributes_[attribute_position] & 0x3f;
                        const int background = (attributes_[attribute_position] >> 6) & 0x3f;

                        render_font(font.get(c), y * character_height, x * 8 * character_width + i * character_width, foreground, background);
                    }
                }
            }
        }
    }

    std::memset(changed_bitmap_, 0, delta_bitmap_size);

    // render cursor

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

        for (int y = 0; y < character_height; y++)
        {
            for (int x = 0; x < character_width; x++)
            {
                set_pixel({.x = cursor_column_ * character_width + x, .y = cursor_row_ * character_height + y}, pixel);
            }
        }
    }

    if (force_trigger_)
    {
        force_trigger_ = false;
        for (int y = 0; y < character_height; y++)
        {
            for (int x = 0; x < character_width; x++)
            {
                set_pixel({.x = cursor_column_ * character_width + x, .y = cursor_row_ * character_height + y}, 63);
            }
        }
    }




}

void Mode80x25::set_pixel(msgui::Position position, int color)
{
    const int pixel_slot = position.y * resolution_width / 5 + position.x / 5; // 5 pixels in uint32_t
    constexpr int bits_per_pixel = 6;
    constexpr int pixels_in_4bytes = 5;
    const int offset = bits_per_pixel * (position.x % pixels_in_4bytes);
    video_ram[pixel_slot] &= ~(0x3f << offset);
    video_ram[pixel_slot] |= (color << offset);
}

int Mode80x25::get_pixel(msgui::Position position)
{
    const int pixel_slot = position.y * resolution_width / 5 + position.x / 5; // 5 pixels in uint32_t
    constexpr int bits_per_pixel = 6;
    constexpr int pixels_in_4bytes = 5;
    const int offset = bits_per_pixel * (position.x % pixels_in_4bytes);
    return video_ram[pixel_slot] &= (0x3f << offset) >> offset;
}

} // namespace text
} // namespace modes
} // namespace vga



