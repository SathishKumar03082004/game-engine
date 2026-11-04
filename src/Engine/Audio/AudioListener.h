#pragma once

#include "../Core/Component.h"

class AudioListener final : public Component
{
public:
    void Update(float deltaTime) override;

protected:
    void OnEnable() override;
    void OnDisable() override;
    void OnDestroy() override;
};
