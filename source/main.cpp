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

#include <cmath>
#include <cstdlib>

#include <board.hpp>

#include <hal/time/sleep.hpp>
#include <stm32f4xx.h>
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_rcc.h>
#include <stm32f4xx_hal_dma.h>
#include <stm32f4xx_hal_tim.h>
#include <stm32f4xx_hal_cortex.h>

#include <hal/interrupt/systick.hpp>

#include <eul/utils/string.hpp>

#include <hal/time/time.hpp>
#include <hal/time/sleep.hpp>

TIM_HandleTypeDef tim2;
TIM_HandleTypeDef tim3;

extern "C"
{
    auto& usart = hal::interfaces::USART_1();


    auto& hsync = hal::gpio::PB4(); // TIM3-CH1
    auto& vsync = hal::gpio::PB6(); // TIM4-CH1

    int counter = 0;
    void TIM3_IRQHandler()
    {
       if(TIM3->SR & TIM_SR_UIF) // if UIF flag is set
        {
            hsync.set_low();
            if (++counter == 640)
            {
                counter = 0;
                vsync.set_low();
                hal::time::sleep(std::chrono::microseconds(60));
                vsync.set_high();

            }
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            hsync.set_high();
            TIM3->SR &= ~TIM_SR_UIF; // clear UIF flag
        }
    }
}

uint32_t line[320];

void draw_line()
{
    for (int i = 0; i < 9000000; ++i)
    {
        GPIOC->ODR = line[i % 320];
    }
}

enum class Polarity
{
    Positive,
    Negative
};

struct Timings
{
    float pixel_frequency;
    struct Line
    {
        const uint32_t visible_pixels;
        const uint32_t front_porch_pixels;
        const uint32_t sync_pulse_pixels;
        const uint32_t back_porch_pixels;
    };

    struct Frame
    {
        const uint32_t visible_lines;
        const uint32_t front_porch_lines;
        const uint32_t sync_pulse_lines;
        const uint32_t back_porch_lines;
    };

    const Line line;
    const Frame frame;
    const Polarity hsync_polarity;
    const Polarity vsync_polarity;
};


constexpr Timings svga_800x600_60 = {
    .pixel_frequency = 40.0,
    .line = {
        .visible_pixels = 800,
        .front_porch_pixels = 40,
        .sync_pulse_pixels = 128,
        .back_porch_pixels = 88
    },
    .frame = {
        .visible_lines = 600,
        .front_porch_lines = 1,
        .sync_pulse_lines = 4,
        .back_porch_lines = 23
    },
    .hsync_polarity = Polarity::Positive,
    .vsync_polarity = Polarity::Negative
};

void initalize_hsync(const Timings& timings)
{
    float clock_to_pixel = HAL_RCC_GetHCLKFreq() / 1000000 / timings.pixel_frequency;
}

int main()
{
    board::board_init();
    hal::interfaces::USART_1().init(9600);

    hal::interrupt::set_systick_handler([](std::chrono::milliseconds){
    });

    hal::time::Time::init();
    hal::interrupt::disable_systick();

    hal::gpio::PC13().init(hal::gpio::Output::PushPull, hal::gpio::Speed::Fast, hal::gpio::PullUpPullDown::Down);
    hsync.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::None);
    vsync.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::None);
    auto& pin = hal::gpio::PC13();

    hsync.set_high();

    initialize_hsync();

    while (true)
    {
    }
}
