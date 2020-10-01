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

#include "modes/modes.hpp"

#include <any>

namespace vga
{

std::string_view to_string(Modes mode)
{
    switch (mode)
    {
        case Modes::Text_66x25: return "Text_66x25";
        case Modes::Graphic_256x240: return "Graphic_256x240";
    }
    return "Unknown";
}

Mode::Mode(vga::Vga& vga)
    : mode_(None{})
    , vga_(vga)
{
}

void Mode::switch_to(const Modes mode)
{
    switch (mode)
    {
        case Modes::Text_66x25:
        {
            mode_ = modes::text::Mode66x25(vga_);
        } break;
    }
}

} // namespace vga
