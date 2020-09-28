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

#include <string_view>

#include "generator/timings.hpp"

namespace vga
{

enum class Mode
{
    Text_80x25 = 0,
    Graphic_256x240 = 1
};

std::string_view to_string(Mode mode);

} // namespace vga



class Vga
{
public:
    void initialize_hsync(const Timings& timings);
    void initialize_vsync(const Timings& timings);
    void setup_draw_function(const vga::Mode mode);
};
