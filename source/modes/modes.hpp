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

#include <variant>

#include "generator/vga.hpp"
#include "modes/text/66x25.hpp"

namespace vga
{

enum class Modes
{
    Text_66x25 = 0,
    Graphic_256x240 = 1
};

std::string_view to_string(Modes mode);

class None
{
};

class Mode
{
public:
    Mode(vga::Vga& vga);

    void switch_to(const Modes mode);
private:
    std::variant<
        None
        , vga::modes::text::Mode66x25
        > mode_;
};

} // namespace vga
