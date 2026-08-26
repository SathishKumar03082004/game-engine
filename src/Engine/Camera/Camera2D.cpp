#include "Camera2D.h"

void Camera2DSystem::Initialize()
{
    camera.target = { 0.0f, 0.0f };
    camera.offset = { 640.0f, 360.0f };

    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

void Camera2DSystem::Update()
{
    float speed = 5.0f;

    if (IsKeyDown(KEY_A))
        camera.target.x -= speed;

    if (IsKeyDown(KEY_D))
        camera.target.x += speed;

    if (IsKeyDown(KEY_W))
        camera.target.y -= speed;

    if (IsKeyDown(KEY_S))
        camera.target.y += speed;

    float wheel = GetMouseWheelMove();

    if (wheel != 0)
    {
        camera.zoom += wheel * 0.1f;

        if (camera.zoom < 0.1f)
            camera.zoom = 0.1f;

        if (camera.zoom > 5.0f)
            camera.zoom = 5.0f;
    }
}

void Camera2DSystem::Begin()
{
    BeginMode2D(camera);
}

void Camera2DSystem::End()
{
    EndMode2D();
}

Camera2D Camera2DSystem::GetCamera()
{
    return camera;
}