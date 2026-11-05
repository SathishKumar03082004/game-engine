#pragma once

#include "../Core/Component.h"

class AudioListener final : public Component
{
public:
    void Update(float deltaTime) override;
    std::unique_ptr<Component> Clone() const override;

protected:
    void OnEnable() override;
    void OnDisable() override;
    void OnDestroy() override;
};
