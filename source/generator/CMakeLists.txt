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

add_library(vga_generator)

target_sources(vga_generator
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/vga.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/timings.hpp

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vga.cpp
)

target_link_libraries(vga_generator PRIVATE hal msgui)

target_compile_options(vga_generator PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)

target_include_directories(vga_generator PUBLIC ${PROJECT_SOURCE_DIR}/source)
