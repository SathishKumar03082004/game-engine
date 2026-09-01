#include "Transform.h"

TransformComponent::TransformComponent(){
    position = {0, 0};
    rotation = 0;
    scale = {1.0f, 1.0f};
}

void TransformComponent::SetPosition(Vector2 newPosition){
    position = newPosition;
}

Vector2 TransformComponent::GetPosition() const{
    return position;
}

void TransformComponent::SetRotation(float newRotation){
    rotation = newRotation;
}

float TransformComponent::GetRotation() const{
    return rotation;
}

void TransformComponent::SetScale(Vector2 newScale){
    scale = newScale;
}

Vector2 TransformComponent::GetScale() const{
    return scale;
}

void TransformComponent::Translate(Vector2 amount){
    position.x += amount.x;
    position.y += amount.y;
}

void TransformComponent::Rotate(float amount){
    rotation += amount;
}