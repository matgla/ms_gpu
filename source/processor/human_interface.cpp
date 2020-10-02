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

#include <algorithm>

#include <eul/mpl/tuples/for_each.hpp>

#include "interfaces/usart.hpp"

namespace processor
{

namespace
{
    Usart usart;
}


template <typename T, typename C>
class Handler
{
public:
    constexpr Handler(std::string_view name, C* object, T data) : handler_(name, std::make_pair(object, data))
    {
    }

    std::pair<std::string_view, std::pair<C*, T>> handler_;
};

template <typename... handlers>
class Handlers
{
public:
    std::tuple<handlers...> handlers_;
};


template <typename... handlers>
Handlers(handlers...) -> Handlers<handlers...>;

HumanInterface::HumanInterface(vga::Mode& mode)
    : position_(0)
    , mode_(&mode)
{
    usart.write("\n> ");
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

    const auto command = get_next_part();

    const static Handlers handlers {
        Handler{"help", this, &HumanInterface::help}
    };

    eul::mpl::tuples::for_each(handlers.handlers_, [&command](const auto& handler) {
        if (command == handler.handler_.first)
        {
            const auto* object = handler.handler_.second.first;
            (object->*handler.handler_.second.second)();
        }
    });
}

void HumanInterface::process(uint8_t byte)
{
    // if (byte == '\r' || byte == '\n')
    // {
    //     process_command();
    //     return;
    // }

    // buffer_[position_] = byte;

    // ++position_;
    // if (position_ == 99)
    // {
    //     process_command();
    //     return;
    // }
    usart.write(static_cast<char>(byte));
    mode_->write(static_cast<char>(byte));

}

void HumanInterface::help() const
{
    usart.write("Available commands:\n");
    usart.write("  mode [mode] - select display mode\n");
    usart.write("To get more information please write help [command].\n");
}

std::string_view HumanInterface::get_next_part()
{
    if (to_parse_.empty())
    {
        return to_parse_;
    }
    to_parse_.remove_prefix(std::min(to_parse_.find_first_not_of(" "), to_parse_.size()));
    const auto next_part = to_parse_.substr(0, std::min(to_parse_.find_first_not_of(" "), to_parse_.size()));
    to_parse_.remove_suffix(next_part.size());
    return next_part;
}

} // namespace processor
