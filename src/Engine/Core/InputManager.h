#pragma once

#include "raylib.h"

#include <array>
#include <string>
#include <unordered_map>

class InputManager
{
public:
    static InputManager& GetInstance();
    void Update();

    bool IsKeyPressed(int key) const;
    bool IsKeyDown(int key) const;
    bool IsKeyReleased(int key) const;
    bool IsMouseButtonPressed(int button) const;
    bool IsMouseButtonDown(int button) const;
    bool IsMouseButtonReleased(int button) const;
    Vector2 GetMousePosition() const;
    float GetMouseWheelMove() const;
    int GetCharPressed() const;

    void BindAction(const std::string& action, int key);
    bool IsActionPressed(const std::string& action) const;
    bool IsActionDown(const std::string& action) const;
    bool IsActionReleased(const std::string& action) const;

private:
    static constexpr int KEY_COUNT = 512;
    static constexpr int MOUSE_COUNT = 8;
    std::array<bool, KEY_COUNT> keyDown{};
    std::array<bool, KEY_COUNT> keyPressed{};
    std::array<bool, KEY_COUNT> keyReleased{};
    std::array<bool, MOUSE_COUNT> mouseDown{};
    std::array<bool, MOUSE_COUNT> mousePressed{};
    std::array<bool, MOUSE_COUNT> mouseReleased{};
    std::unordered_map<std::string, int> actionKeys;
    Vector2 mousePosition{};
    float mouseWheel = 0.0f;
};
