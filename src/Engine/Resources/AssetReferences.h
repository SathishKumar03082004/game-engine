#pragma once

#include "AssetReference.h"

namespace AssetReferences
{
    inline const AssetReference PlayerSprite{
        "sprite.player",
        "resources/sprites/player.png",
        AssetType::TextureAsset
    };

    inline const AssetReference EnemySprite{
        "sprite.enemy",
        "resources/sprites/enemy.png",
        AssetType::TextureAsset
    };

    inline const AssetReference PlatformSprite{
        "sprite.platform",
        "resources/sprites/platform.png",
        AssetType::TextureAsset
    };

    inline const AssetReference PlayerAnimationSheet{
        "animation.player.sheet",
        "resources/sprites/player_walk.png",
        AssetType::TextureAsset
    };
}
