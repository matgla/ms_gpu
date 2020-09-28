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

#include "processor/human_interface.hpp"

#include "interfaces/usart.hpp"

namespace processor
{

namespace
{
    Usart usart;
}

HumanInterface::HumanInterface()
    : position_(0)
{
    usart.write("> ");
}

void HumanInterface::process_command()
{
    usart.write('\n');
    usart.write("Processing command: ");
    buffer_[position_] = 0;
    usart.write(buffer_);
    usart.write('\n');
    position_ = 0;

    to_parse_ = buffer_;

}

void HumanInterface::process(uint8_t byte)
{
    if (byte == '\r' || byte == '\n')
    {
        process_command();
        return;
    }

    buffer_[position_] = byte;

    ++position_;
    if (position_ == 99)
    {
        process_command();
        return;
    }
    usart.write(byte);
}

std::string_view HumanInterface::get_part() const
{


}

} // namespace processor
