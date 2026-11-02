
#pragma once

enum class RenderLayer : int
{
    Background = 0,
    Default = 1,
    Characters = 2,
    Foreground = 3,
    UI = 4
};

inline const char* GetRenderLayerName(RenderLayer layer)
{
    switch (layer)
    {
        case RenderLayer::Background: return "Background";
        case RenderLayer::Default: return "Default";
        case RenderLayer::Characters: return "Characters";
        case RenderLayer::Foreground: return "Foreground";
        case RenderLayer::UI: return "UI";
        default: return "Default";
    }
}
