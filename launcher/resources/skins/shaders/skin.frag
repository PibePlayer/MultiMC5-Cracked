#version 150 core

/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

in vec2 texCoord;
flat in int texID;
flat in int texTransparency;

out vec4 finalColor;

uniform sampler2D skinTexture;
uniform sampler2D capeTexture;

float epsilon = 1.0 / 255.0;

void main() {
    vec4 color;
    if (texID == 0)
    {
        color = texture(skinTexture, texCoord);
    }
    else
    {
        color = texture(capeTexture, texCoord);
    }
    if(texTransparency == 0)
    {
        // replace transparency with black. TODO: verify this is correct
        if(color.w < epsilon)
        {
            finalColor = vec4(0.0, 0.0, 0.0, 1.0);
        }
        else
        {
            finalColor = color;
            finalColor.w = 1.0;
        }
    }
    else
    {
        // full transparency -> discard
        if(color.w < epsilon)
        {
            discard;
        }
        finalColor = color;
    }
}
