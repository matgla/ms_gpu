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

find_package(msgui REQUIRED)

add_subdirectory(generator)
add_subdirectory(interfaces)
add_subdirectory(memory)
add_subdirectory(modes)
add_subdirectory(processor)

target_sources(msgpu
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/fpga_connection.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/syscalls.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/fpga_connection.cpp
)

target_link_libraries(msgpu PRIVATE
    hal
    msgui
    vga_generator
    msgpu_interfaces
    msgpu_modes
    msgpu_processor
)

target_compile_options(msgpu PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a;-Wno-volatile>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)

target_include_directories(msgpu PUBLIC ${PROJECT_SOURCE_DIR})

if (${vendor} STREQUAL "STM32")
    set_target_properties(msgpu
        PROPERTIES
            SUFFIX ".elf"
    )
endif ()

include (GenerateBinary)
generate_binary(msgpu ${vendor})
