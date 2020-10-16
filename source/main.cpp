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
#include <stm32f4xx_hal_tim_ex.h>


#include <hal/time/time.hpp>
#include <hal/time/sleep.hpp>
#include <hal/interrupt/systick.hpp>

#include <eul/utils/string.hpp>

#include "generator/vga.hpp"
#include "generator/timings.hpp"
#include "interfaces/usart.hpp"
#include "interfaces/spi.hpp"
#include "modes/modes.hpp"
#include "memory/video_ram.hpp"

#include "processor/command_processor.hpp"

#include "fpga_connection.hpp"
#include <hal/core/core.hpp>

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

constexpr static uint32_t bootloader_address = 0x1FFF0000;
constexpr static uint32_t ram_address_for_bootloader = 0x2001FFFF;

void jump_to_bootloader()
{
    __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();
    __set_MSP(ram_address_for_bootloader);
    // void (*bootloader)(void) = reinterpret_cast<(void(*)(void))>(bootloader_address + 4);
    // bootloader();
}

int main()
{
    board::board_init();

    hal::core::Core::initializeClocks();

    hal::interrupt::set_systick_handler([](std::chrono::milliseconds){
    });


    hal::interrupt::set_systick_period(std::chrono::milliseconds(100));

    Usart usart;
    usart.initialize();

    // constexpr auto font = msgui::fonts::Font5x7::data;

    // int pos_x = 2;

    // int escape_counter = 0;
    // bool human_interface = false;
    // processor::CommandProcessor processor(mode);
    // processor.change();

    // Spi::initialize();

    usart.write("START\n");
    interface::FpgaConnection fpga;
    GPIO_InitTypeDef gpio;
    gpio.Pin = GPIO_PIN_8;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF1_TIM1;


    HAL_GPIO_Init(GPIOA, &gpio);


    fpga.init();

    uint8_t data[] = { 0xff};
    uint8_t data2[] = { 0x00};

    uint8_t msg[100];



    while (true)
    {
        if (fpga.ready_for_transmission())
        {
            usart.write("TRANSMIT B\n");
            fpga.transmit_data(data);
            hal::time::sleep(std::chrono::milliseconds(500));

        }
        if (fpga.ready_for_transmission())
        {
            hal::time::sleep(std::chrono::milliseconds(500));
            fpga.transmit_data(data2);
        }
    }
}
