
#pragma once

#include "raylib.h"
#include "../../Engine/Scene/Scene.h"

class RenderSettingsPanel
{
public:
    explicit RenderSettingsPanel(Scene& scene);

    void Initialize();
    void Update();
    void Draw();

    bool IsMouseOver() const;

private:
    Scene& scene;
    Rectangle panel;

    static constexpr int MIN_SORTING_ORDER = -1000;
    static constexpr int MAX_SORTING_ORDER = 1000;

    void DrawButton(
        Rectangle rect,
        const char* label,
        bool hovered
    );
};
