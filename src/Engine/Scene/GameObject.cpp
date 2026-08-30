#include "GameObject.h"

GameObject::GameObject(){
    position = {0.0f, 0.0f};
    rotation = 0.0f;
    scale = {1.0f, 1.0f};

    selected = false;

    size = {100.0f, 100.0f};
}

void GameObject::Update(){

}

void GameObject::Draw(){
    float width = size.x * scale.x;
    float height = size.y * scale.y;

    Rectangle rectangle = {position.x - width/2.0f, position.y - height/2.0f, width, height};

    DrawRectanglePro(rectangle, {width/2.0f, height/2.0f}, rotation, GRAY);

    if(selected){
        DrawRectangleLinesEx(rectangle, 3.0f, BLUE);
    }
}

void GameObject::SetPosition(Vector2 newPosition){
    position = newPosition;
}

void GameObject::SetRotation(float newRotation){
    rotation = newRotation;
}

void GameObject::SetScale(Vector2 newScale){
    scale = newScale;
}

Vector2 GameObject::GetPosition() const{
    return position;
}

float GameObject::GetRotation() const{
    return rotation;
}

Vector2 GameObject::GetScale() const{
    return scale;
}

void GameObject::SetSelected(bool newSelected){
    selected = newSelected;
}

bool GameObject::IsSelected() const{
    return selected;
}