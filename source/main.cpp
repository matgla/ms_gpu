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

#include <stm32f4xx.h>
#include <stm32f4xx_hal.h>

#include <board.hpp>

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

    hal::interrupt::set_systick_handler([](std::chrono::milliseconds){
    });

    hal::time::Time::init();
    hal::interrupt::disable_systick();
    char data[99];

    eul::utils::itoa(HAL_RCC_GetHCLKFreq(), data);
    hal::interfaces::USART_1().write(data);
    hal::interfaces::USART_1().write("\n");
    Vga vga;
    vga.initialize_hsync(svga_800x600_60);
    vga.initialize_vsync(svga_800x600_60);
    auto& pa2 = hal::gpio::PA2();
    pa2.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Up);
    auto& pa4 = hal::gpio::PA4();
    pa4.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Up);
    while (true)
    {
        GPIOA->ODR = 0x4;
        GPIOA->ODR = 0x0;
    }
}
