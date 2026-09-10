#include "GameObject.h"


GameObject::GameObject()
{
    selected = false;

    size =
    {
        100.0f,
        100.0f
    };

    name = "GameObject";
}


// ============================================================
// UPDATE
// ============================================================

void GameObject::Update()
{
}


// ============================================================
// DRAW
// ============================================================

void GameObject::Draw()
{
    Vector2 position =
        transform.GetPosition();


    Vector2 scale =
        transform.GetScale();


    float rotation =
        transform.GetRotation();


    float width =
        size.x * scale.x;


    float height =
        size.y * scale.y;


    Rectangle rectangle =
    {
        position.x,
        position.y,
        width,
        height
    };


    Vector2 origin =
    {
        width / 2.0f,
        height / 2.0f
    };


    DrawRectanglePro(
        rectangle,
        origin,
        rotation,
        GRAY
    );


    // --------------------------------------------------------
    // Selection outline
    // --------------------------------------------------------

    if (selected)
    {
        DrawRectangleLinesEx(
            rectangle,
            3.0f,
            BLUE
        );
    }
}


// ============================================================
// GET TRANSFORM
// ============================================================

TransformComponent&
GameObject::GetTransform()
{
    return transform;
}


// ============================================================
// SELECTION
// ============================================================

void GameObject::SetSelected(
    bool newSelected
)
{
    selected = newSelected;
}


bool GameObject::IsSelected() const
{
    return selected;
}


// ============================================================
// CONTAINS POINT
// ============================================================

bool GameObject::ContainsPoint(
    Vector2 worldPoint
) const
{
    Vector2 position =
        transform.GetPosition();


    Vector2 scale =
        transform.GetScale();


    float width =
        size.x * scale.x;


    float height =
        size.y * scale.y;


    Rectangle rectangle =
    {
        position.x - width / 2.0f,
        position.y - height / 2.0f,
        width,
        height
    };


    return CheckCollisionPointRec(
        worldPoint,
        rectangle
    );
}


void GameObject::SetName(const std::string& newName){
    name = newName;
}

const std::string&GameObject::GetName() const{
    return name;
}