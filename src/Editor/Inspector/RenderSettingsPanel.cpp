
#include "RenderSettingsPanel.h"
#include <string>

RenderSettingsPanel::RenderSettingsPanel(Scene& sceneReference)
    : scene(sceneReference)
{
    panel = {
        300.0f,
        15.0f,
        330.0f,
        125.0f
    };
}

void RenderSettingsPanel::Initialize()
{
}

bool RenderSettingsPanel::IsMouseOver() const
{
    return CheckCollisionPointRec(
        GetMousePosition(),
        panel
    );
}

void RenderSettingsPanel::DrawButton(
    Rectangle rect,
    const char* label,
    bool hovered
)
{
    DrawRectangleRec(
        rect,
        hovered ? SKYBLUE : DARKGRAY
    );

    DrawRectangleLinesEx(
        rect,
        1.0f,
        WHITE
    );

    const int textWidth = MeasureText(label, 16);

    DrawText(
        label,
        static_cast<int>(
            rect.x + (rect.width - textWidth) * 0.5f
        ),
        static_cast<int>(rect.y + 7.0f),
        16,
        WHITE
    );
}

void RenderSettingsPanel::Update()
{
    GameObject* object = scene.GetSelectedObject();

    if (object == nullptr)
    {
        return;
    }

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        return;
    }

    Vector2 mouse = GetMousePosition();

    if (!CheckCollisionPointRec(mouse, panel))
    {
        return;
    }

    Rectangle previousLayerButton = {
        panel.x + 15.0f,
        panel.y + 42.0f,
        35.0f,
        30.0f
    };

    Rectangle nextLayerButton = {
        panel.x + 280.0f,
        panel.y + 42.0f,
        35.0f,
        30.0f
    };

    Rectangle decreaseOrderButton = {
        panel.x + 15.0f,
        panel.y + 85.0f,
        35.0f,
        30.0f
    };

    Rectangle increaseOrderButton = {
        panel.x + 280.0f,
        panel.y + 85.0f,
        35.0f,
        30.0f
    };

    int currentLayer = static_cast<int>(
        object->GetRenderLayer()
    );

    if (CheckCollisionPointRec(mouse, previousLayerButton))
    {
        currentLayer = (currentLayer + 4) % 5;

        object->SetRenderLayer(
            static_cast<RenderLayer>(currentLayer)
        );

        return;
    }

    if (CheckCollisionPointRec(mouse, nextLayerButton))
    {
        currentLayer = (currentLayer + 1) % 5;

        object->SetRenderLayer(
            static_cast<RenderLayer>(currentLayer)
        );

        return;
    }

    int order = object->GetSortingOrder();

    if (CheckCollisionPointRec(mouse, decreaseOrderButton))
    {
        if (order > MIN_SORTING_ORDER)
        {
            object->SetSortingOrder(order - 1);
        }

        return;
    }

    if (CheckCollisionPointRec(mouse, increaseOrderButton))
    {
        if (order < MAX_SORTING_ORDER)
        {
            object->SetSortingOrder(order + 1);
        }

        return;
    }
}

void RenderSettingsPanel::Draw()
{
    DrawRectangleRec(panel, Color{35, 35, 35, 245});
    DrawRectangleLinesEx(panel, 1.0f, WHITE);

    DrawText(
        "RENDER SETTINGS",
        static_cast<int>(panel.x + 15.0f),
        static_cast<int>(panel.y + 12.0f),
        18,
        WHITE
    );

    GameObject* object = scene.GetSelectedObject();

    if (object == nullptr)
    {
        DrawText(
            "Select a GameObject",
            static_cast<int>(panel.x + 15.0f),
            static_cast<int>(panel.y + 65.0f),
            16,
            LIGHTGRAY
        );

        return;
    }

    Rectangle previousLayerButton = {
        panel.x + 15.0f,
        panel.y + 42.0f,
        35.0f,
        30.0f
    };

    Rectangle nextLayerButton = {
        panel.x + 280.0f,
        panel.y + 42.0f,
        35.0f,
        30.0f
    };

    Rectangle decreaseOrderButton = {
        panel.x + 15.0f,
        panel.y + 85.0f,
        35.0f,
        30.0f
    };

    Rectangle increaseOrderButton = {
        panel.x + 280.0f,
        panel.y + 85.0f,
        35.0f,
        30.0f
    };

    Vector2 mouse = GetMousePosition();

    DrawButton(
        previousLayerButton,
        "<",
        CheckCollisionPointRec(mouse, previousLayerButton)
    );

    DrawButton(
        nextLayerButton,
        ">",
        CheckCollisionPointRec(mouse, nextLayerButton)
    );

    const char* layerName = GetRenderLayerName(
        object->GetRenderLayer()
    );

    const int layerTextWidth = MeasureText(layerName, 16);

    DrawText(
        layerName,
        static_cast<int>(
            panel.x + (panel.width - layerTextWidth) * 0.5f
        ),
        static_cast<int>(panel.y + 49.0f),
        16,
        YELLOW
    );

    DrawText(
        "Layer",
        static_cast<int>(panel.x + 65.0f),
        static_cast<int>(panel.y + 49.0f),
        14,
        LIGHTGRAY
    );

    DrawButton(
        decreaseOrderButton,
        "-",
        CheckCollisionPointRec(mouse, decreaseOrderButton)
    );

    DrawButton(
        increaseOrderButton,
        "+",
        CheckCollisionPointRec(mouse, increaseOrderButton)
    );

    const std::string orderText =
        std::to_string(object->GetSortingOrder());

    const int orderTextWidth = MeasureText(orderText.c_str(), 18);

    DrawText(
        orderText.c_str(),
        static_cast<int>(
            panel.x + (panel.width - orderTextWidth) * 0.5f
        ),
        static_cast<int>(panel.y + 91.0f),
        18,
        GREEN
    );

    DrawText(
        "Order",
        static_cast<int>(panel.x + 65.0f),
        static_cast<int>(panel.y + 93.0f),
        14,
        LIGHTGRAY
    );
}
