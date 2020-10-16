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

#include "interfaces/spi.hpp"

#include <cstring>

#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_spi.h>

#include "interfaces/usart.hpp"

namespace
{
    uint8_t rx_buffer_[100];
    SPI_HandleTypeDef spi;
}

void Spi::initialize()
{
    // GPU is slave unit on SPI bus

    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio;
    gpio.Pin =  GPIO_PIN_14;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_HIGH;
    gpio.Alternate = GPIO_AF5_SPI2;

    HAL_GPIO_Init(GPIOB, &gpio);

    gpio.Pin = GPIO_PIN_15 | GPIO_PIN_13;
    gpio.Mode = GPIO_MODE_AF_OD;
    HAL_GPIO_Init(GPIOB, &gpio);

    gpio.Pin = GPIO_PIN_12;
    gpio.Mode = GPIO_MODE_AF_OD;
    HAL_GPIO_Init(GPIOB, &gpio);

    spi.Instance = SPI2;
    spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    spi.Init.Direction = SPI_DIRECTION_2LINES;
    spi.Init.CLKPhase = SPI_PHASE_2EDGE;
    spi.Init.CLKPolarity = SPI_POLARITY_LOW;
    spi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    spi.Init.CRCPolynomial = 7;
    spi.Init.DataSize = SPI_DATASIZE_8BIT;
    spi.Init.FirstBit = SPI_FIRSTBIT_LSB;
    spi.Init.NSS = SPI_NSS_HARD_INPUT;
    spi.Init.TIMode = SPI_TIMODE_DISABLE;
    spi.Init.Mode = SPI_MODE_SLAVE;
    HAL_SPI_Init(&spi);

    std::memset(rx_buffer_, 0, 100);
}

void Spi::write(const DataStream& data)
{
    HAL_SPI_Transmit(&spi, data.data(), data.size(), 1000);
}

void Spi::write(uint8_t data)
{
    HAL_SPI_Transmit(&spi, &data, 1, 1000);
}

uint8_t Spi::read()
{
    while (true)
    {
        uint8_t d;
        HAL_SPI_Receive(&spi, &d, 1, HAL_MAX_DELAY);
        Usart::write("rec: ");
        Usart::write(d);
        Usart::write("\n");
    }
    // uint8_t data;
    // if (HAL_OK == HAL_SPI_Receive(&spi, &data, 1, 1000))
    // {
    //     return data;
    // }

    // uint8_t input[16], output = 0x7f;


    // for (;;)
    // {
    //     uint8_t ready = 0xe7;
    //     HAL_SPI_Transmit(&spi, &ready, 1, HAL_MAX_DELAY);
    //     HAL_SPI_Transmit(&spi, &ready, 1, HAL_MAX_DELAY);
    //     HAL_SPI_Receive(&spi, (uint8_t *)&input, 1, HAL_MAX_DELAY);
    //     HAL_SPI_Receive(&spi, (uint8_t *)&input, 16, HAL_MAX_DELAY);
    //     bool correct_data = true;

    //     for (int i = 0; i < 16; ++i)
    //     {
    //         Usart::write("recv: ");
    //         Usart::write(input[i]);
    //         Usart::write("\n");
    //         if (i >= 1)
    //         {
    //             if (input[i] - input[i - 1] != 1)
    //             {
    //                 correct_data = false;
    //             }
    //         }
    //     }

    //     if (correct_data)
    //     {
    //          output = 0x7f;
    //         Usart::write("Respond ack\n");
    //         HAL_SPI_Transmit(&spi, &output, 1, 1000);
    //         Usart::write("responded ack\n");
    //     }
    //     else
    //     {
    //          output = 0x00;
    //         Usart::write("Receive failure\n");
    //         HAL_SPI_Transmit(&spi, &output, 1, 1000);

    //     }


    // }

    return {};
}
