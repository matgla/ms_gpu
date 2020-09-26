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


.syntax unified
.arch armv7-m
.thumb

.global draw_800
draw_800:
    // prepare 
    push {r2}
    
    @ .rept 100
        @ .set pixel_index, 0
        .rept 400
        // first pixel

        ldrbt r2, [r0, #1] // load first 4 pixels // 2C
        strb r2, [r1]                // store to odr // 2C
        add r0, #1
        // but with ART this is eual to 4 clock ticks
        @ nop                          // align to offset count // 1C
        @ .set pixel_index, pixel_index + 1
        .endr
        @ add r0, #4                   // move to next array element 1C  
    @ .endr
    // clear at the end
    mov r2, #0 // 1C
    nop
    strb r2, [r1] // 2C

    pop {r2}
    bx lr 

