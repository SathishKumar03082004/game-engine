
#include "RenderSettingsPanel.h"
#include <string>

RenderSettingsPanel::RenderSettingsPanel(Scene& sceneReference)
    : scene(sceneReference)
{
    panel = {
        300.0f,
        15.0f,
        330.0f,
        205.0f
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
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        return;
    }

    Vector2 mouse = GetMousePosition();

    if (!CheckCollisionPointRec(mouse, panel))
    {
        return;
    }

    const Rectangle decreaseGridButton = {panel.x + 15.0f, panel.y + 142.0f, 35.0f, 28.0f};
    const Rectangle increaseGridButton = {panel.x + 280.0f, panel.y + 142.0f, 35.0f, 28.0f};
    const Rectangle snappingButton = {panel.x + 15.0f, panel.y + 175.0f, 300.0f, 24.0f};
    if (CheckCollisionPointRec(mouse, decreaseGridButton))
    {
        scene.SetGridSize(scene.GetGridSize() - 8);
        return;
    }
    if (CheckCollisionPointRec(mouse, increaseGridButton))
    {
        scene.SetGridSize(scene.GetGridSize() + 8);
        return;
    }
    if (CheckCollisionPointRec(mouse, snappingButton))
    {
        scene.SetGridSnappingEnabled(!scene.IsGridSnappingEnabled());
        return;
    }

    GameObject* object = scene.GetSelectedObject();
    if (object == nullptr) return;

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

    DrawText("Grid Size", static_cast<int>(panel.x + 15.0f), static_cast<int>(panel.y + 124.0f), 12, LIGHTGRAY);
    Rectangle decreaseGridButton = {panel.x + 15.0f, panel.y + 142.0f, 35.0f, 28.0f};
    Rectangle increaseGridButton = {panel.x + 280.0f, panel.y + 142.0f, 35.0f, 28.0f};
    const Vector2 mouse = GetMousePosition();
    DrawButton(decreaseGridButton, "-", CheckCollisionPointRec(mouse, decreaseGridButton));
    DrawButton(increaseGridButton, "+", CheckCollisionPointRec(mouse, increaseGridButton));
    const std::string gridText = std::to_string(scene.GetGridSize()) + " px";
    const int gridTextWidth = MeasureText(gridText.c_str(), 16);
    DrawText(gridText.c_str(), static_cast<int>(panel.x + (panel.width - gridTextWidth) * 0.5f),
        static_cast<int>(panel.y + 148.0f), 16, YELLOW);

    const Rectangle snappingButton = {panel.x + 15.0f, panel.y + 175.0f, 300.0f, 24.0f};
    DrawRectangleRec(snappingButton, scene.IsGridSnappingEnabled() ? DARKGREEN : DARKGRAY);
    DrawRectangleLinesEx(snappingButton, 1.0f, WHITE);
    const char* snappingLabel = scene.IsGridSnappingEnabled() ? "Grid Snap: ON" : "Grid Snap: OFF";
    const int snappingTextWidth = MeasureText(snappingLabel, 13);
    DrawText(snappingLabel, static_cast<int>(snappingButton.x + (snappingButton.width - snappingTextWidth) * 0.5f),
        static_cast<int>(snappingButton.y + 6.0f), 13, WHITE);

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
