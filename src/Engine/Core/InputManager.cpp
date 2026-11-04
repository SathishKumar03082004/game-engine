#include "InputManager.h"

InputManager& InputManager::GetInstance()
{
    static InputManager manager;
    return manager;
}

void InputManager::Update()
{
    for (int key = 0; key < KEY_COUNT; ++key)
    {
        const bool down = ::IsKeyDown(key);
        keyPressed[key] = down && !keyDown[key];
        keyReleased[key] = !down && keyDown[key];
        keyDown[key] = down;
    }
    for (int button = 0; button < MOUSE_COUNT; ++button)
    {
        const bool down = ::IsMouseButtonDown(button);
        mousePressed[button] = down && !mouseDown[button];
        mouseReleased[button] = !down && mouseDown[button];
        mouseDown[button] = down;
    }
    mousePosition = ::GetMousePosition();
    mouseWheel = ::GetMouseWheelMove();
}

bool InputManager::IsKeyPressed(int key) const { return key >= 0 && key < KEY_COUNT && keyPressed[key]; }
bool InputManager::IsKeyDown(int key) const { return key >= 0 && key < KEY_COUNT && keyDown[key]; }
bool InputManager::IsKeyReleased(int key) const { return key >= 0 && key < KEY_COUNT && keyReleased[key]; }
bool InputManager::IsMouseButtonPressed(int button) const { return button >= 0 && button < MOUSE_COUNT && mousePressed[button]; }
bool InputManager::IsMouseButtonDown(int button) const { return button >= 0 && button < MOUSE_COUNT && mouseDown[button]; }
bool InputManager::IsMouseButtonReleased(int button) const { return button >= 0 && button < MOUSE_COUNT && mouseReleased[button]; }
Vector2 InputManager::GetMousePosition() const { return mousePosition; }
float InputManager::GetMouseWheelMove() const { return mouseWheel; }
int InputManager::GetCharPressed() const { return ::GetCharPressed(); }
void InputManager::BindAction(const std::string& action, int key) { if (!action.empty() && key >= 0 && key < KEY_COUNT) actionKeys[action] = key; }
bool InputManager::IsActionPressed(const std::string& action) const
{
    const auto found = actionKeys.find(action);
    return found != actionKeys.end() && IsKeyPressed(found->second);
}
bool InputManager::IsActionDown(const std::string& action) const
{
    const auto found = actionKeys.find(action);
    return found != actionKeys.end() && IsKeyDown(found->second);
}
bool InputManager::IsActionReleased(const std::string& action) const
{
    const auto found = actionKeys.find(action);
    return found != actionKeys.end() && IsKeyReleased(found->second);
}
