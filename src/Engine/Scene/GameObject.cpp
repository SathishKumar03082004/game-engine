#include "GameObject.h"

GameObject::GameObject(){

    selected = false;

    size = {100.0f, 100.0f};
}

void GameObject::Update(){

}

void GameObject::Draw(){
    Vector2 position = transform.GetPosition();
    Vector2 scale = transform.GetScale();
    float rotation = transform.GetRotation();

    float width = size.x * scale.x;
    float height = size.y * scale.y;

    Rectangle rectangle = {
        position.x,
        position.y,
        width,
        height
    };

    Vector2 origin = {
        width / 2.0f,
        height / 2.0f
    };

    DrawRectanglePro(rectangle, origin, rotation, GRAY);

    if(selected){
        DrawRectangleLinesEx(rectangle, 3.0f, BLUE);
    }
}

TransformComponent& GameObject::GetTransform(){
    return transform;
}

void GameObject::SetSelected(bool newSelected){
    selected = newSelected;
}

bool GameObject::IsSelected() const{
    return selected;
}