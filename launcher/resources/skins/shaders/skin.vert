#version 150 core

/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

// vertex shader

in vec3 position;
in vec2 texcoords;
in int texnr;
in int transparency;

out vec2 texCoord;
flat out int texID;
flat out int texTransparency;

uniform mat4 worldToView;

void main() {
    gl_Position = worldToView * vec4(position, 1.0);
    texCoord = texcoords;
    texID = texnr;
    texTransparency = transparency;
}
