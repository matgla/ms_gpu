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

#include "fpga_connection.hpp"

#include <board.hpp>

#include <stm32f4xx.h>
#include <stm32f4xx_hal.h>

#include <eul/function.hpp>

#include "arm/stm32/stm32f4xx/gpio/stm32f4xx_gpio.hpp"
#include "interfaces/usart.hpp"

namespace interface
{

static auto& data_0 = hal::gpio::PB0();
static auto& data_1 = hal::gpio::PB1();
static auto& data_2 = hal::gpio::PB2();
static auto& data_3 = hal::gpio::PB3();
static auto& data_4 = hal::gpio::PB4();
static auto& data_5 = hal::gpio::PB5();
static auto& data_6 = hal::gpio::PB6();
static auto& data_7 = hal::gpio::PB7();

static auto& clock = hal::gpio::PA8();
static auto& en = hal::gpio::PA0();
static auto& cd = hal::gpio::PA1(); // command or data

const auto gpio_odr = reinterpret_cast<uint32_t>(&GPIOB->ODR);

namespace
{

TIM_HandleTypeDef tim;
DMA_HandleTypeDef dma;

enum class PortState : uint8_t
{
    None,
    Input,
    Output
};

enum class State : uint8_t
{
    Idle,
    TrassmissionOngoing
};

static PortState port_state_;
static volatile State state_;

} // namespace

extern "C"
{

void DMA2_Stream1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&dma);
}

}

void FpgaConnection::init()
{
    //clock.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Up);
    initialize_timer();
    initialize_dma();

    clock.init(hal::gpio::Alternate::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Up);
    static_cast<hal::gpio::DigitalInputOutputPin::Impl*>(&clock)
        ->set_alternate_function(GPIO_AF1_TIM1);

    en.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Down);
    cd.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Down);

    set_as_output(); // this is master interface, so it will initiate communication
}

void FpgaConnection::set_as_input()
{
    if (port_state_ == PortState::Input)
    {
        return;
    }
    data_0.init(hal::gpio::Input::Floating, hal::gpio::PullUpPullDown::None);
    data_1.init(hal::gpio::Input::Floating, hal::gpio::PullUpPullDown::None);
    data_2.init(hal::gpio::Input::Floating, hal::gpio::PullUpPullDown::None);
    data_3.init(hal::gpio::Input::Floating, hal::gpio::PullUpPullDown::None);
    data_4.init(hal::gpio::Input::Floating, hal::gpio::PullUpPullDown::None);
    data_5.init(hal::gpio::Input::Floating, hal::gpio::PullUpPullDown::None);
    data_6.init(hal::gpio::Input::Floating, hal::gpio::PullUpPullDown::None);
    data_7.init(hal::gpio::Input::Floating, hal::gpio::PullUpPullDown::None);

    // rw.set_low();
}

void FpgaConnection::set_as_output()
{
    if (port_state_ == PortState::Output)
    {
        return;
    }

    data_0.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Down);
    data_1.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Down);
    data_2.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Down);
    data_3.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Down);
    data_4.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Down);
    data_5.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Down);
    data_6.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Down);
    data_7.init(hal::gpio::Output::PushPull, hal::gpio::Speed::High, hal::gpio::PullUpPullDown::Down);

    // rw.set_high();
}

void FpgaConnection::initialize_dma()
{
    __HAL_RCC_DMA2_CLK_ENABLE();

    dma.Instance = DMA2_Stream1;
    dma.Init.Channel = DMA_CHANNEL_6;
    dma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dma.Init.PeriphInc = DMA_PINC_DISABLE;
    dma.Init.MemInc = DMA_MINC_ENABLE;
    dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma.Init.Mode = DMA_NORMAL;
    dma.Init.Priority = DMA_PRIORITY_HIGH;
    dma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

    HAL_DMA_Init(&dma);

    __HAL_LINKDMA(&tim, hdma[TIM_DMA_ID_CC1], dma);
    HAL_DMA_Init(tim.hdma[TIM_DMA_ID_CC1]);

    HAL_DMA_RegisterCallback(&dma, HAL_DMA_XFER_CPLT_CB_ID, &FpgaConnection::transmission_finished);
}

void FpgaConnection::initialize_timer()
{
    // TODO: remove hardcode and allow to create different frequencies
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    uint16_t prescaler = (uint16_t) (SystemCoreClock / 1000000) - 1;

    TIM1->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS);
    TIM1->CR1 |= TIM_COUNTERMODE_UP;
    TIM1->CR1 &= ~TIM_CR1_CKD;
    TIM1->CR1 |= TIM_CLOCKDIVISION_DIV1;
    TIM1->ARR = 5 - 1; 
    TIM1->CCR1 = 2; 
    TIM1->PSC = 1; 
    TIM1->RCR = 10 - 1; // function to setup 
    TIM1->EGR = TIM_EGR_UG;
    TIM1->SMCR = RESET;
    TIM1->CR1 |= TIM_CR1_OPM;
    //TIM1->BDTR = RESET;
    //TIM1->BDTR |= TIM_LOCKLEVEL_OFF;
   // //TIM1->BDTR |= TIM_OSSI_ENABLE;
   // //TIM1->BDTR |= TIM_OSSR_ENABLE;
   // //TIM1->BDTR |= TIM_BREAK_ENABLE;
   // //TIM1->BDTR |= TIM_BREAKPOLARITY_HIGH;
   // //TIM1->BDTR |= TIM_AUTOMATICOUTPUT_ENABLE;
   //TIM1->CR1 |= TIM_CR1_OPM;
   TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
   TIM1->CCMR1 &= ~TIM_CCMR1_CC1S;
   TIM1->CCMR1 |= TIM_OCMODE_PWM1;
   TIM1->CCER &= ~TIM_CCER_CC1P;
   TIM1->CCER |= TIM_OCPOLARITY_HIGH;
   TIM1->CCER = TIM_CCER_CC1E;
   TIM1->BDTR |= TIM_BDTR_MOE;
   //TIM1->CR1 |= TIM_CR1_CEN;
    // __HAL_RCC_TIM1_CLK_ENABLE();
    tim.Instance = TIM1;
    tim.Init.Prescaler = 1;
    tim.Init.CounterMode = TIM_COUNTERMODE_UP;
    tim.Init.Period = 100 - 1;
    tim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    tim.Init.RepetitionCounter = 0;
    tim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

}

bool FpgaConnection::transmit_data(const gsl::span<uint8_t>& data)
{
    if (state_ != State::Idle)
    {
        return false;
    }

    cd.set_high();
    start_dma_transmission(data);
    return true;
}

bool FpgaConnection::transmit_command(const gsl::span<uint8_t>& data)
{
    if (state_ != State::Idle)
    {
        return false;
    }

    cd.set_low();
    start_dma_transmission(data);
    return true;
}

bool FpgaConnection::ready_for_transmission()
{
    return state_ == State::Idle;
}

static int counter = 0;

void FpgaConnection::transmission_finished(DMA_HandleTypeDef* hdma)
{
    __HAL_TIM_DISABLE_DMA(&tim, TIM_DMA_CC1);
    //HAL_TIM_PWM_Stop(&tim, TIM_CHANNEL_1);
    //__HAL_TIM_DISABLE(&tim);
    GPIOB->ODR = 0x00;

    // Usart::write("Finished\n");
    state_ = State::Idle;
}

void FpgaConnection::start_dma_transmission(const gsl::span<uint8_t>& data)
{
//    Usart::write("Start transmission\n");

    HAL_DMA_Start_IT(tim.hdma[TIM_DMA_ID_CC1], reinterpret_cast<uint32_t>(data.data()), (uint32_t)&GPIOB->ODR, data.size());
 //   Usart::write("Start transmission 2\n");
    state_ = State::TrassmissionOngoing;
    __HAL_TIM_ENABLE_DMA(&tim, TIM_DMA_CC1);
  //  __HAL_TIM_ENABLE(&tim);
   // TIM1->CNT = 0; 
    
    TIM1->RCR = data.size() - 1; // function to setup 
    TIM1->EGR |= TIM_EGR_UG;

    TIM1->CR1 |= TIM_CR1_CEN;
    //HAL_TIM_OnePulse_Start(&tim, TIM_CHANNEL_1);
}


} // namespace interface


