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

#include <chrono>

#include <board.hpp>

#include <stm32f4xx_hal.h>

#include <hal/time/time.hpp>
#include <hal/time/sleep.hpp>
#include <hal/interrupt/systick.hpp>

#include <eul/utils/string.hpp>

#include "vga.hpp"
#include "timings.hpp"

int main()
{
    board::board_init();
    hal::interfaces::USART_1().init(9600);
    auto& usart = hal::interfaces::USART_1();
    hal::interfaces::USART_1().write("---gpu---\n");
    hal::interrupt::set_systick_handler([](std::chrono::milliseconds){
    });

    hal::time::Time::init();
    hal::interrupt::disable_systick();
    char data[99];

    eul::utils::itoa(HAL_RCC_GetHCLKFreq(), data);
    hal::interfaces::USART_1().write(data);
    hal::interfaces::USART_1().write("\n");
    Vga vga;
    vga.setup_draw_function();
    vga.initialize_hsync(svga_800x600_60);
    vga.initialize_vsync(svga_800x600_60);
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

    while (true)
    {
        usart.write("This is not a blocker\n");
    }
}
