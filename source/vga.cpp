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

#include "vga.hpp"

#include <cstring>

#include <stm32f4xx.h> 
#include <stm32f4xx_hal.h> 
#include <stm32f4xx_hal_rcc.h> 
#include <stm32f4xx_hal_dma.h>  // tim.h has dependency to dma
#include <stm32f4xx_hal_tim.h> 
#include <arm/stm32/stm32f4xx/gpio/stm32f4xx_gpio.hpp>

#include <board.hpp>

#include "image.hpp"

namespace
{

auto& hsync = hal::gpio::PB4(); // TIM3-CH1
auto& vsync = hal::gpio::PB6(); // TIM4-CH1
auto& usart = hal::interfaces::USART_1();

TIM_HandleTypeDef tim2;
TIM_HandleTypeDef tim3;
TIM_HandleTypeDef tim4;

} // namespace

extern "C"
{
    void TIM2_IRQHandler()
    {
        if (__HAL_TIM_GET_FLAG(&tim2, TIM_FLAG_CC2) != RESET 
            && __HAL_TIM_GET_IT_SOURCE(&tim2, TIM_FLAG_CC2) != RESET)
        {
            __HAL_TIM_CLEAR_IT(&tim2, TIM_FLAG_CC2);
            asm volatile inline ("wfi\n");
        }
        else
        {
            HAL_TIM_IRQHandler(&tim2);
        }
    }

    // const uint8_t data[] = {0xC, 0xC, 0xC0, 0xC, 0xC, 0xC0, 0xC, 0xC, 0xC0, 0xC, 0xC, 0xC0, 0xC, 0xC, 0xC0, 0xC, 0xC, 0xC0, 0xC, 0xC, 0xC0, 0xC, 0xC, 0xC0, 0xC, 0xC, 0xC0, 0xC, 0xC, 0xC0, 0xC, 0xC, 0xC0, 0xC, 0xC, 0xC0, 0xC, 0xC, 0xC0};
    // uint32_t data_2[100] = {};

    void draw_800(const uint32_t* data, volatile uint32_t* odr);
    void draw_256(const uint32_t* data, volatile uint32_t* odr);

    void draw_800_wrapper(const uint32_t* data, volatile uint32_t* odr)
    {
        draw_800(data, odr);
    }

    void draw_256_wrapper(const uint32_t* data, volatile uint32_t* odr)
    {
        draw_256(data, odr);
    }

    static void(*draw_func)(const uint32_t*, volatile uint32_t*) = &draw_800_wrapper;
    int line_counter = 0;
    int image_line = 0;
    int empty_lines = 60;

    inline void draw() 
    {
        if (empty_lines != 0) return;
        if (image_line == 240) return;
        draw_func(&data[image_line * 64], static_cast<volatile uint32_t*>(&GPIOA->ODR));
    }

    bool is_vsync = false;
    // after back porch 
    void TIM3_IRQHandler()
    {

        if (__HAL_TIM_GET_FLAG(&tim3, TIM_FLAG_CC2) != RESET 
            && __HAL_TIM_GET_IT_SOURCE(&tim3, TIM_FLAG_CC2) != RESET)
        {
            if (!is_vsync)
            {
                draw();
                if (empty_lines)
                {
                    --empty_lines;
                }
                else 
                {
                    if (line_counter % 2)
                    {
                        if (image_line < 240)
                        {
                            ++image_line;
                        }
                    }
                }
            }
            if (++line_counter == 598)
            {
                is_vsync = true;
            }
            __HAL_TIM_CLEAR_IT(&tim3, TIM_FLAG_CC2);
        }
        else
        {
            HAL_TIM_IRQHandler(&tim3);
        }
    }

    void TIM4_IRQHandler()
    {

        if (__HAL_TIM_GET_FLAG(&tim4, TIM_FLAG_CC4) != RESET 
            && __HAL_TIM_GET_IT_SOURCE(&tim4, TIM_FLAG_CC4) != RESET)
        {

            line_counter = 0;
            is_vsync = false;
            image_line =0;
            empty_lines = 60;
            __HAL_TIM_CLEAR_IT(&tim4, TIM_FLAG_CC4);
        }
        else
        {
            HAL_TIM_IRQHandler(&tim4);
        }
    }

}

void Vga::setup_draw_function()
{
    draw_func = &draw_256_wrapper;
}

void Vga::initialize_hsync(const Timings& timings)
{
    const float clocks_in_pixel = HAL_RCC_GetHCLKFreq() / 1000000 / timings.pixel_frequency;
    hsync.init(hal::gpio::Alternate::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Up);
    
    static_cast<hal::gpio::DigitalInputOutputPin::Impl*>(&hsync)
        ->set_alternate_function(GPIO_AF2_TIM3);


    const int ticks_for_line = clocks_in_pixel * 
        (timings.line.visible_pixels + timings.line.back_porch_pixels 
        + timings.line.sync_pulse_pixels + timings.line.front_porch_pixels)
        - 1;

    // timer 2 is to prevent random latency on interrupt
    // inspired by http://cliffle.com/blog/glitch-in-the-matrix/

    __HAL_RCC_TIM2_CLK_ENABLE();

    tim2.Instance = TIM2;
    tim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    tim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    tim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    tim2.Init.Period = ticks_for_line;
    tim2.Init.Prescaler = 0;
    HAL_TIM_Base_Init(&tim2);

    TIM_OC_InitTypeDef oc;
    TIM_MasterConfigTypeDef master;

    master.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
    master.MasterOutputTrigger = TIM_TRGO_ENABLE;
    HAL_TIMEx_MasterConfigSynchronization(&tim2, &master);

    HAL_NVIC_SetPriority(TIM2_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    oc.OCMode = TIM_OCMODE_INACTIVE;
    oc.Pulse = 100; // test value 
    HAL_TIM_OC_ConfigChannel(&tim2, &oc, TIM_CHANNEL_2);
    __HAL_TIM_ENABLE_IT(&tim2, TIM_IT_CC2);

    __HAL_RCC_TIM3_CLK_ENABLE();
    tim3.Instance = TIM3;
    tim3.Init.Prescaler = 0;
    tim3.Init.Period = ticks_for_line;
    tim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    tim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    tim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&tim3);

    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);

    oc.OCMode = timings.hsync_polarity == Polarity::Negative ? TIM_OCMODE_PWM2 : TIM_OCMODE_PWM1;
    const int sync_time =  timings.line.sync_pulse_pixels * clocks_in_pixel - 1;
    oc.Pulse = sync_time;
    oc.OCIdleState = TIM_OCIDLESTATE_RESET;
    oc.OCPolarity = TIM_OCPOLARITY_LOW;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&tim3, &oc, TIM_CHANNEL_1);
    TIM_CCxChannelCmd(tim3.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);

    oc.OCMode = TIM_OCMODE_INACTIVE;
    const int back_porch_time = clocks_in_pixel * timings.line.back_porch_pixels + sync_time;
    const int delay_for_draw_start = 46;
    oc.Pulse = back_porch_time - delay_for_draw_start;
    HAL_TIM_OC_ConfigChannel(&tim3, &oc, TIM_CHANNEL_2);
    __HAL_TIM_ENABLE_IT(&tim3, TIM_IT_CC2);

    master.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
    master.MasterOutputTrigger = TIM_TRGO_UPDATE;
    HAL_TIMEx_MasterConfigSynchronization(&tim3, &master);

    TIM_SlaveConfigTypeDef slave;
    slave.InputTrigger = TIM_TS_ITR1;
    slave.SlaveMode = TIM_SLAVEMODE_TRIGGER;
    slave.TriggerFilter = 0;
    slave.TriggerPolarity = TIM_TRIGGERPOLARITY_NONINVERTED;
    slave.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
    HAL_TIM_SlaveConfigSynchronization(&tim3, &slave);

    __HAL_TIM_SET_COUNTER(&tim2, 0);
    __HAL_TIM_SET_COUNTER(&tim3, 0);
    // HAL_TIM_PWM_Start(&tim3, TIM_CHANNEL_1);
    __HAL_TIM_ENABLE(&tim2);
    __HAL_TIM_ENABLE(&tim3);
}

void Vga::initialize_vsync(const Timings& timings)
{
    // Timer 4 for vsync counts Timer 3 pulses 
    const int frame = timings.frame.back_porch_lines
        + timings.frame.front_porch_lines 
        + timings.frame.sync_pulse_lines 
        + timings.frame.visible_lines; 

    vsync.init(hal::gpio::Alternate::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Up);
    
    static_cast<hal::gpio::DigitalInputOutputPin::Impl*>(&vsync)
        ->set_alternate_function(GPIO_AF2_TIM4);

     __HAL_RCC_TIM4_CLK_ENABLE();
    tim4.Instance = TIM4;
    tim4.Init.Prescaler = 0;
    tim4.Init.Period = frame - 1;
    tim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    tim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    tim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&tim4);

    HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);

    TIM_SlaveConfigTypeDef slave;
    slave.SlaveMode = TIM_SLAVEMODE_GATED;
    slave.InputTrigger = TIM_TS_ITR2;
    slave.TriggerPolarity = TIM_TRIGGERPOLARITY_NONINVERTED;
    slave.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
    slave.TriggerFilter = 0;
    HAL_TIM_SlaveConfigSynchronization(&tim4, &slave);

    TIM_OC_InitTypeDef oc;
    oc.OCMode = timings.hsync_polarity == Polarity::Negative ? TIM_OCMODE_PWM2 : TIM_OCMODE_PWM1;
    oc.Pulse = timings.frame.sync_pulse_lines;
    oc.OCIdleState = TIM_OCIDLESTATE_RESET;
    oc.OCPolarity = TIM_OCPOLARITY_LOW;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&tim4, &oc, TIM_CHANNEL_1);

    oc.OCMode = TIM_OCMODE_INACTIVE;
    oc.Pulse = timings.frame.sync_pulse_lines + timings.frame.back_porch_lines - 2;
    HAL_TIM_OC_ConfigChannel(&tim4, &oc, TIM_CHANNEL_4);
    __HAL_TIM_ENABLE_IT(&tim4, TIM_IT_CC4);

    TIM_CCxChannelCmd(tim4.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);
    __HAL_TIM_ENABLE(&tim4);
}
