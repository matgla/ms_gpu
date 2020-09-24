# This file is part of MS GPU project.
# Copyright (C) 2020 Mateusz Stadnik
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


add_executable(msgpu)

target_sources(msgpu
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/timings.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vga.hpp
        
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/syscalls.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vga.cpp
)

target_link_libraries(msgpu PRIVATE hal)


if (${vendor} STREQUAL "STM32")
    set_target_properties(msgpu
        PROPERTIES
            SUFFIX ".elf"
    )
endif ()

include (GenerateBinary)
generate_binary(msgpu ${vendor})