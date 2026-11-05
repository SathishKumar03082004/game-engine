
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
    void SetOpen(bool value) { open = value; }
    bool IsOpen() const { return open; }

private:
    Scene& scene;
    Rectangle panel;
    bool open = false;

    static constexpr int MIN_SORTING_ORDER = -1000;
    static constexpr int MAX_SORTING_ORDER = 1000;

    void DrawButton(
        Rectangle rect,
        const char* label,
        bool hovered
    );
};
