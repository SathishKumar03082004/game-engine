#include "Renderer2D.h"

#include <raymath.h>

void Renderer2D::BeginFrame()
{
    commands.clear();
    stats = {};
}
void Renderer2D::SubmitSprite(Texture2D* texture, Rectangle source, Rectangle destination,
    Vector2 origin, float rotation, Color tint)
{
    if (texture == nullptr || texture->id == 0 || source.width == 0.0f || source.height == 0.0f) return;
    commands.emplace_back(SpriteCommand{texture, source, destination, origin, rotation, tint});
    ++stats.submittedSprites;
}
void Renderer2D::SubmitRectangle(Rectangle rectangle, Vector2 origin, float rotation, Color color)
{
    commands.emplace_back(RectangleCommand{rectangle, origin, rotation, color});
}
void Renderer2D::SubmitLine(Vector2 start, Vector2 end, float thickness, Color color)
{
    commands.emplace_back(LineCommand{start, end, thickness, color});
}
void Renderer2D::SubmitCircle(Vector2 center, float radius, Color color)
{
    if (radius > 0.0f) commands.emplace_back(CircleCommand{center, radius, color});
}
void Renderer2D::Flush()
{
    Texture2D* previousTexture = nullptr;
    for (const Command& command : commands)
    {
        if (const auto* sprite = std::get_if<SpriteCommand>(&command))
        {
            if (sprite->texture != previousTexture)
            {
                ++stats.spriteBatches;
                ++stats.drawCalls;
                previousTexture = sprite->texture;
            }
            DrawTexturePro(*sprite->texture, sprite->source, sprite->destination,
                sprite->origin, sprite->rotation, sprite->tint);
        }
        else
        {
            previousTexture = nullptr;
            ++stats.drawCalls;
            if (const auto* rectangle = std::get_if<RectangleCommand>(&command))
                DrawRectanglePro(rectangle->rectangle, rectangle->origin, rectangle->rotation, rectangle->color);
            else if (const auto* line = std::get_if<LineCommand>(&command))
                DrawLineEx(line->start, line->end, line->thickness, line->color);
            else if (const auto* circle = std::get_if<CircleCommand>(&command))
                DrawCircleV(circle->center, circle->radius, circle->color);
        }
    }
    commands.clear();
}
const RendererStats& Renderer2D::GetStats() const { return stats; }
