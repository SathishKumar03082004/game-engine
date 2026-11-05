#pragma once
#include "raylib.h"

class PlayModeToolbar
{
public:
    enum class State { Stopped, Playing, Paused };
    enum class Action { None, Play, Pause, Stop, Step };
    Action Update(State state);
    void Draw(State state) const;
    bool IsMouseOver() const;
private:
    Rectangle GetButton(int index) const;
};
