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

#include "stm32f401ceu6_usart.hpp"

#include <stm32f4xx.h>
#include <stm32f4xx_hal_rcc.h>
#include <stm32f4xx_hal_gpio.h>
#include <stm32f4xx_hal_dma.h>
#include <stm32f4xx_hal_uart.h>
#include <stm32f4xx_hal_usart.h>


#include <eul/utils/string.hpp>

#include <hal/time/sleep.hpp>

namespace
{
    static UART_HandleTypeDef usart;

} // namespace

void Usart::initialize()
{
    hal::interfaces::USART_1().init(9600);
}

uint8_t buffer[50];

void Usart::write(const std::string_view& msg)
{
    hal::interfaces::USART_1().write(msg);
}

void Usart::write(char c)
{
    hal::interfaces::USART_1().write(std::string_view(&c, 1));
}

void Usart::write(int n)
{
    char buf[100];
    eul::utils::itoa(n, buf, 10);
    hal::interfaces::USART_1().write(std::string_view(buf));
}

void Usart::write_hex(int n)
{
    char buf[100];
    eul::utils::itoa(n, buf, 16);
    hal::interfaces::USART_1().write(std::string_view(buf));
}


std::string_view Usart::read()
{
    int i = 0;
    int timeout = 0;
    while (i < 49)
    {
        while (__HAL_UART_GET_FLAG(&usart, UART_FLAG_RXNE) == RESET)
        {
            if (++timeout > 100000)
            {
                buffer[i] = 0;
                return reinterpret_cast<const char*>(buffer);
            }
        }
        buffer[i] = USART1->DR & 0xff;
        ++i;
    }

    buffer[i] = 0;
    return reinterpret_cast<const char*>(buffer);
}
