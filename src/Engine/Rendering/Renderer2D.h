#pragma once

#include "raylib.h"

#include <variant>
#include <vector>

struct RendererStats
{
    int drawCalls = 0;
    int spriteBatches = 0;
    int submittedSprites = 0;
};

class Renderer2D
{
public:
    void BeginFrame();
    void SubmitSprite(Texture2D* texture, Rectangle source, Rectangle destination,
        Vector2 origin, float rotation, Color tint);
    void SubmitRectangle(Rectangle rectangle, Vector2 origin, float rotation, Color color);
    void SubmitLine(Vector2 start, Vector2 end, float thickness, Color color);
    void SubmitCircle(Vector2 center, float radius, Color color);
    void Flush();
    const RendererStats& GetStats() const;

private:
    struct SpriteCommand { Texture2D* texture; Rectangle source; Rectangle destination; Vector2 origin; float rotation; Color tint; };
    struct RectangleCommand { Rectangle rectangle; Vector2 origin; float rotation; Color color; };
    struct LineCommand { Vector2 start; Vector2 end; float thickness; Color color; };
    struct CircleCommand { Vector2 center; float radius; Color color; };
    using Command = std::variant<SpriteCommand, RectangleCommand, LineCommand, CircleCommand>;
    std::vector<Command> commands;
    RendererStats stats;
};
