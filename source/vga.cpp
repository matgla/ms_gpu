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

#include <stm32f4xx.h> 
#include <stm32f4xx_hal.h> 
#include <stm32f4xx_hal_rcc.h> 
#include <stm32f4xx_hal_dma.h>  // tim.h has dependency to dma
#include <stm32f4xx_hal_tim.h> 

#include <arm/stm32/stm32f4xx/gpio/stm32f4xx_gpio.hpp>

#include <board.hpp>

namespace
{

auto& hsync = hal::gpio::PB4(); // TIM3-CH1
auto& vsync = hal::gpio::PB6(); // TIM4-CH1

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
        }
        else
        {
            HAL_TIM_IRQHandler(&tim2);
        }
    }

    void TIM3_IRQHandler()
    {
        if (__HAL_TIM_GET_FLAG(&tim3, TIM_FLAG_CC2) != RESET 
            && __HAL_TIM_GET_IT_SOURCE(&tim3, TIM_FLAG_CC2) != RESET)
        {
            __HAL_TIM_CLEAR_IT(&tim3, TIM_FLAG_CC2);
        }
        else
        {
            HAL_TIM_IRQHandler(&tim3);
        }
    }

    void TIM4_IRQHandler()
    {
        if (__HAL_TIM_GET_FLAG(&tim4, TIM_FLAG_CC2) != RESET 
            && __HAL_TIM_GET_IT_SOURCE(&tim4, TIM_FLAG_CC2) != RESET)
        {
            __HAL_TIM_CLEAR_IT(&tim4, TIM_FLAG_CC2);
        }
        else
        {
            HAL_TIM_IRQHandler(&tim4);
        }
    }
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
    __HAL_RCC_TIM3_CLK_ENABLE();
    tim3.Instance = TIM3;
    tim3.Init.Prescaler = 0;
    tim3.Init.Period = ticks_for_line;
    tim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    tim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    tim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&tim3);

    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);

    TIM_OC_InitTypeDef oc;
    oc.OCMode = timings.hsync_polarity == Polarity::Negative ? TIM_OCMODE_PWM2 : TIM_OCMODE_PWM1;
    oc.Pulse = timings.line.sync_pulse_pixels * clocks_in_pixel - 1;
    oc.OCIdleState = TIM_OCIDLESTATE_RESET;
    oc.OCPolarity = TIM_OCPOLARITY_LOW;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&tim3, &oc, TIM_CHANNEL_1);

    oc.OCMode = TIM_OCMODE_INACTIVE;
    oc.Pulse = 200 - 1;
    HAL_TIM_OC_ConfigChannel(&tim3, &oc, TIM_CHANNEL_2);

    TIM_MasterConfigTypeDef master;
    master.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
    master.MasterOutputTrigger = TIM_TRGO_UPDATE;
    HAL_TIMEx_MasterConfigSynchronization(&tim3, &master);

    HAL_TIM_Base_Start_IT(&tim3);
    HAL_TIM_PWM_Start(&tim3, TIM_CHANNEL_1);
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
    HAL_TIM_Base_Start_IT(&tim4);
    // HAL_TIM_PWM_Start(&tim4, TIM_CHANNEL_1);

    TIM_CCxChannelCmd(tim4.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);
}