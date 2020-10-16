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

#pragma once

#include <cstdint>
#include <gsl/span>

extern "C"
{
    struct __DMA_HandleTypeDef;
    typedef __DMA_HandleTypeDef DMA_HandleTypeDef;
}

namespace interface
{

class FpgaConnection
{
public:
    static void init();

    static bool transmit_data(const gsl::span<uint8_t>& data);
    static bool transmit_command(const gsl::span<uint8_t>& data);
    static bool ready_for_transmission();
private:
    static void set_as_input();
    static void set_as_output();
    static void initialize_timer();
    static void initialize_dma();
    static void start_dma_transmission(const gsl::span<uint8_t>& data);
    static void transmission_finished(DMA_HandleTypeDef* hdma);

};

} // namespace interface
