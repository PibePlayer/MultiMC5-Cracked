#version 150 core

/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

in vec2 position;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
}

