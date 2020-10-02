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

#include "interfaces/usart.hpp"

#include <stm32f4xx.h>
#include <stm32f4xx_hal_rcc.h>
#include <stm32f4xx_hal_gpio.h>
#include <stm32f4xx_hal_dma.h>
#include <stm32f4xx_hal_uart.h>
#include <stm32f4xx_hal_usart.h>

#include <eul/utils/string.hpp>

namespace
{
    static UART_HandleTypeDef usart;

} // namespace

void Usart::initialize()
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitTypeDef  gpio;

    gpio.Pin       = GPIO_PIN_6;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FAST;
    gpio.Alternate = GPIO_AF7_USART1;

    HAL_GPIO_Init(GPIOB, &gpio);

    /* UART RX GPIO pin configuration  */
    gpio.Pin = GPIO_PIN_7;
    gpio.Alternate = GPIO_AF7_USART1;

    HAL_GPIO_Init(GPIOB, &gpio);

    usart.Instance = USART1;
    usart.Init.BaudRate = 112500;
    usart.Init.WordLength   = UART_WORDLENGTH_8B;
    usart.Init.StopBits     = UART_STOPBITS_1;
    usart.Init.Parity       = UART_PARITY_NONE;
    usart.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    usart.Init.Mode         = UART_MODE_TX_RX;
    usart.Init.OverSampling = UART_OVERSAMPLING_8;

    HAL_UART_Init(&usart);
}

void Usart::write(const std::string_view& msg)
{
    HAL_UART_Transmit(&usart, reinterpret_cast<uint8_t*>(const_cast<char*>(msg.data())), msg.length(), 100);
}

void Usart::write(char c)
{
    HAL_UART_Transmit(&usart, reinterpret_cast<uint8_t*>(&c), 1, 100);
}

void Usart::write(int n)
{
    char buf[100];
    eul::utils::itoa(n, buf, 10);
    Usart::write(buf);
}

void Usart::write_hex(int n)
{
    char buf[100];
    eul::utils::itoa(n, buf, 10);
    Usart::write(buf);
}


std::optional<char> Usart::read()
{
    if(__HAL_UART_GET_FLAG(&usart, UART_FLAG_RXNE) == SET)
    {
        return USART1->DR;
    }
    return {};
}
