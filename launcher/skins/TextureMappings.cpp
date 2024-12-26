/* Copyright 2025 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#include "TextureMappings.h"

namespace Skins {

const TextureMapping capeLayout[2] = {
    {
        { 22, 2, 2, 32 },
        { 0, 2, 2, 32 },
        { 2, 0, 20, 2 },
        { 22, 0, 20, 2, true },
        { 2, 2, 20, 32 },
        { 24, 2, 20, 32 },
        Texture::Cape,
        true
    },
    {
        { 11, 1, 1, 16 },
        { 0, 1, 1, 16 },
        { 1, 0, 10, 1 },
        { 11, 0, 10, 1, true },
        { 1, 1, 10, 16 },
        { 12, 1, 10, 16 },
        Texture::Cape,
        true
    }
};

const TextureMapping head = {
    { 16, 8, 8, 8 },
    { 0, 8, 8, 8 },
    { 8, 0, 8, 8 },
    { 16, 0, 8, 8, true },
    { 8, 8, 8, 8 },
    { 24, 8, 8, 8 },
    Texture::Skin,
    false
};

const TextureMapping head_cover = {
    { 48, 8, 8, 8 },
    { 32, 8, 8, 8 },
    { 40, 0, 8, 8 },
    { 48, 0, 8, 8, true },
    { 40, 8, 8, 8 },
    { 56, 8, 8, 8 },
    Texture::Skin,
    true
};

const TextureMapping torso = {
    { 28, 20, 4, 12 },
    { 16, 20, 4, 12 },
    { 20, 16, 8, 4 },
    { 28, 16, 8, 4, true },
    { 20, 20, 8, 12 },
    { 32, 20, 8, 12 },
    Texture::Skin,
    false
};

const TextureMapping torso_cover = {
    { 28, 36, 4, 12 },
    { 16, 36, 4, 12 },
    { 20, 32, 8, 4 },
    { 28, 32, 8, 4, true },
    { 20, 36, 8, 12 },
    { 32, 36, 8, 12 },
    Texture::Skin,
    true
};

const TextureMapping left_arm_old_classic = {
    { 40, 20, 4, 12, false, true },
    { 48, 20, 4, 12, false, true },
    { 44, 16, 4, 4, false, true},
    { 48, 16, 4, 4, true, true },
    { 44, 20, 4, 12, false, true },
    { 52, 20, 4, 12, false, true },
    Texture::Skin,
    false
};

const TextureMapping left_arm_old_slim = {
    { 40, 20, 4, 12, false, true },
    { 47, 20, 4, 12, false, true },
    { 44, 16, 3, 4, false, true },
    { 47, 16, 3, 4, true, true },
    { 44, 20, 3, 12, false, true },
    { 51, 20, 3, 12, false, true },
    Texture::Skin,
    false
};

const TextureMapping left_arm_classic = {
    { 40, 52, 4, 12 },
    { 32, 52, 4, 12 },
    { 36, 48, 4, 4 },
    { 40, 48, 4, 4, true },
    { 36, 52, 4, 12 },
    { 44, 52, 4, 12 },
    Texture::Skin,
    false
};

const TextureMapping left_arm_slim = {
    { 39, 52, 4, 12 },
    { 32, 52, 4, 12 },
    { 36, 48, 3, 4 },
    { 39, 48, 3, 4, true },
    { 36, 52, 3, 12 },
    { 43, 52, 3, 12 },
    Texture::Skin,
    false
};

const TextureMapping left_arm_cover_classic = {
    { 56, 52, 4, 12 },
    { 48, 52, 4, 12 },
    { 52, 48, 4, 4 },
    { 56, 48, 4, 4, true },
    { 52, 52, 4, 12 },
    { 60, 52, 4, 12 },
    Texture::Skin,
    true
};

const TextureMapping left_arm_cover_slim = {
    { 55, 52, 4, 12 },
    { 48, 52, 4, 12 },
    { 52, 48, 3, 4 },
    { 55, 48, 3, 4, true },
    { 52, 52, 3, 12 },
    { 59, 52, 3, 12 },
    Texture::Skin,
    true
};

const TextureMapping right_arm_old_classic = {
    { 48, 20, 4, 12 },
    { 40, 20, 4, 12 },
    { 44, 16, 4, 4 },
    { 48, 16, 4, 4, true },
    { 44, 20, 4, 12 },
    { 52, 20, 4, 12 },
    Texture::Skin,
    false
};

const TextureMapping right_arm_old_slim = {
    { 47, 20, 4, 12 },
    { 40, 20, 4, 12 },
    { 44, 16, 3, 4 },
    { 47, 16, 3, 4, true },
    { 44, 20, 3, 12 },
    { 51, 20, 3, 12 },
    Texture::Skin,
    false
};

const TextureMapping right_arm_classic = {
    { 48, 20, 4, 12 },
    { 40, 20, 4, 12 },
    { 44, 16, 4, 4 },
    { 48, 16, 4, 4, true },
    { 44, 20, 4, 12 },
    { 52, 20, 4, 12 },
    Texture::Skin,
    false
};

const TextureMapping right_arm_slim = {
    { 47, 20, 4, 12 },
    { 40, 20, 4, 12 },
    { 44, 16, 3, 4 },
    { 47, 16, 3, 4, true },
    { 44, 20, 3, 12 },
    { 51, 20, 3, 12 },
    Texture::Skin,
    false
};

const TextureMapping right_arm_cover_classic = {
    { 48, 36, 4, 12 },
    { 40, 36, 4, 12 },
    { 44, 32, 4, 4 },
    { 48, 32, 4, 4, true },
    { 44, 36, 4, 12 },
    { 52, 36, 4, 12 },
    Texture::Skin,
    true
};

const TextureMapping right_arm_cover_slim = {
    { 47, 36, 4, 12 },
    { 40, 36, 4, 12 },
    { 44, 32, 3, 4 },
    { 47, 32, 3, 4, true },
    { 44, 36, 3, 12 },
    { 51, 36, 3, 12 },
    Texture::Skin,
    true
};


const TextureMapping right_leg = {
    { 8, 20, 4, 12 },
    { 0, 20, 4, 12 },
    { 4, 16, 4, 4 },
    { 8, 16, 4, 4, true },
    { 4, 20, 4, 12 },
    { 12, 20, 4, 12 },
    Texture::Skin,
    false
};

const TextureMapping right_leg_cover = {
    { 8, 36, 4, 12 },
    { 0, 36, 4, 12 },
    { 4, 32, 4, 4 },
    { 8, 32, 4, 4, true },
    { 4, 36, 4, 12 },
    { 12, 36, 4, 12 },
    Texture::Skin,
    true
};

const TextureMapping left_leg_old = {
    { 0, 20, 4, 12, false, true},
    { 8, 20, 4, 12, false, true},
    { 4, 16, 4, 4, false, true},
    { 8, 16, 4, 4, true, true},
    { 4, 20, 4, 12, false, true},
    { 12, 20, 4, 12, false, true},
    Texture::Skin,
    false
};

const TextureMapping left_leg = {
    { 24, 52, 4, 12 },
    { 16, 52, 4, 12 },
    { 20, 48, 4, 4 },
    { 24, 48, 4, 4, true },
    { 20, 52, 4, 12 },
    { 28, 52, 4, 12 },
    Texture::Skin,
    false
};

const TextureMapping left_leg_cover = {
    { 8, 52, 4, 12 },
    { 0, 52, 4, 12 },
    { 4, 48, 4, 4 },
    { 8, 48, 4, 4, true },
    { 4, 52, 4, 12 },
    { 12, 52, 4, 12 },
    Texture::Skin,
    true
};

}

