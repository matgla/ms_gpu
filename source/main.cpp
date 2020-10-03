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

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>

#include <board.hpp>

void nanosleep(timespec* ts, timespec *tc)
{

}

#include <msgui/fonts/Font5x7.hpp>
#include <msgui/Factory.hpp>

#include <stm32f4xx_hal.h>


#include <hal/time/time.hpp>
#include <hal/time/sleep.hpp>
#include <hal/interrupt/systick.hpp>

#include <eul/utils/string.hpp>

#include "generator/vga.hpp"
#include "generator/timings.hpp"
#include "interfaces/usart.hpp"
#include "modes/modes.hpp"
#include "memory/video_ram.hpp"

#include "processor/command_processor.hpp"

void initalize_video_pins()
{
    auto& pa0 = hal::gpio::PA0();
    auto& pa1 = hal::gpio::PA1();
    auto& pa2 = hal::gpio::PA2();
    auto& pa3 = hal::gpio::PA3();
    auto& pa4 = hal::gpio::PA4();
    auto& pa5 = hal::gpio::PA5();


    pa0.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Up);
    pa1.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Up);
    pa2.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Up);
    pa3.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Up);
    pa4.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Up);
    pa5.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Up);

}

std::string_view get_argument(std::string_view command)
{
    const auto delimiter = command.find(' ');
    if (delimiter != std::string_view::npos)
    {
        return command.substr(delimiter + 1, command.size());
    }
    return "";
}

bool has_command(std::string_view str, std::string_view command)
{
    const auto command_position = str.find(command);
    if (command_position != std::string_view::npos)
    {
        return true;
    }
    return false;
}

int main()
{
    board::board_init();
    hal::interrupt::set_systick_handler([](std::chrono::milliseconds){
    });

    hal::time::Time::init();
    // hal::interrupt::s
    hal::interrupt::disable_systick();

    Usart usart;
    usart.initialize();

    vga::Vga vga;
    vga::Mode mode(vga);
    mode.switch_to(vga::Modes::Text_80x25);
    vga.initialize_hsync(svga_800x600_60);
    vga.initialize_vsync(svga_800x600_60);

    initalize_video_pins();

    constexpr auto font = msgui::fonts::Font5x7::data;

    int pos_x = 2;

    int escape_counter = 0;
    bool human_interface = false;
    processor::CommandProcessor processor(mode);
    processor.change();

    while (true)
    {
        if (vga.render())
        {
            mode.render();
            vga.render(false);
            // asm volatile inline ("wfi\n");
        }
        // for (int i = 0; i < 1000; ++i)
        // {
            // hal::time::sleep(std::chrono::microseconds(1000));
        // }
        auto data = usart.read();

        for (char byte : data)
        {
            if (byte == 27)
            {
                ++escape_counter;
                if (escape_counter == 3)
                {
                    processor.change();
                    escape_counter = 0;
                }
                processor.process(byte);
                continue;
            }
            escape_counter = 0;
            processor.process(byte);
        }

        /* command.remove_prefix(std::min(command.find_first_not_of(" "), command.size()));
        command.remove_suffix(command.size() - command.find_last_not_of(" ") - 1);
        const auto help_position = command.find("help");
        if (help_position != std::string_view::npos)
        {
            if (command.length() == 4)
            {
                usart.write("Available commands:\n");
                usart.write("  mode [mode] - select display mode\n");
                usart.write("To get more information please write help [command].\n");
            }
            else
            {
            }
            continue;
        }

        if (has_command(command, "mode"))
        {
            const auto delimiter = command.find(' ');
            const auto argument = get_argument(command);
            if (!argument.empty())
            {
                const auto mode = command.substr(delimiter + 1, command.size());
                usart.write("Switch to mode: ");
                vga::Mode mode_number(static_cast<vga::Mode>(std::atoi(mode.data())));
                usart.write(vga::to_string(mode_number));
                usart.write("\n");
                vga.setup_draw_function(mode_number);
            }
            continue;
        }

        if (has_command(command, "newline"))
        {
            const auto delimiter = command.find(' ');
            const auto argument = get_argument(command);
            if (!argument.empty())
            {
                const auto mode = command.substr(delimiter + 1, command.size());
                usart.write("Switch to mode: ");
                vga::Mode mode_number(static_cast<vga::Mode>(std::atoi(mode.data())));
                usart.write(vga::to_string(mode_number));
                usart.write("\n");
                vga.setup_draw_function(mode_number);
            }
            continue;
        }

        if (has_command(command, "clear"))
        {
            const auto argument = get_argument(command);
            int color = std::atoi(argument.data());
            // vga.clear(color);
            pos_x = 0;
            for (int i = 0; i < 256; ++i)
            {
                // vga.set_pixel(0, i, 63);
                // vga.set_pixel(239, i, 63);
                // vga.set_pixel(i, 0, 63);
                // vga.set_pixel(i, 255, 63);
            }

            continue;
        }

        if (has_command(command, ";"))
        {
            const auto y_delimiter = command.find(';');
            const auto y_data = command.substr(0, y_delimiter);
            auto rest =  command.substr(y_delimiter + 1, command.size());
            usart.write(rest);
            usart.write("\n");
            char buffer[4];
            std::memcpy(buffer, y_data.data(), y_data.size());
            buffer[y_data.size()] = 0;
            int y = std::atoi(buffer);

            const auto x_delimiter = command.find(';');
            const auto x_data = rest.substr(0, x_delimiter);
            rest =  rest.substr(x_delimiter + 1, rest.size());
            std::memcpy(buffer, x_data.data(), x_data.size());
            buffer[x_data.size()] = 0;
            usart.write(rest);
            usart.write("\n");
            int x = std::atoi(buffer);
            int color = std::atoi(rest.data());
            // vga.set_pixel(y, x, color);
        }
        else
        {
            for (const auto c : command)
            {
                const auto& ch = font.get(c);
                for (int y = 0; y < ch.height(); ++y)
                {
                    for (int x = 0; x < ch.width(); ++x)
                    {
                        if (ch.getPixel(x, y))
                        {
                            // vga.set_pixel(y + 2, x + pos_x, 12);
                        }
                    }
                }
                pos_x += ch.width() + 1;
            }

        }
        // usart.write("This is not a blocker\n");*/
    }
}
