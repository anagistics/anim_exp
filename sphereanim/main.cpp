// main.cpp
#include "raylib.h"
#include <vector>
#include <random>
#include <cmath>

// Structure to represent a sphere with position and velocity
struct Sphere {
    Vector3 position;
    Vector3 velocity;
};

constexpr float CUBE_BOUND = 10.0f;

class Scene {
private:
    std::vector<Sphere> spheres;
    float connectionThreshold;
    float sphereRadius;

    // Constants for appearance
    const Color SPHERE_COLOR = BLUE;
    const Color LINE_COLOR1 = BLUE;
    const Color LINE_COLOR2 = DARKBLUE;
    const float MIN_SPEED = 0.01f;
    const float MAX_SPEED = 0.03f;

public:
    Scene(int numSpheres, float threshold)
        : connectionThreshold(threshold), sphereRadius(0.2f)
    {
        // Random number generation
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posDist(-CUBE_BOUND / 2.0, CUBE_BOUND / 2.0);
        std::uniform_real_distribution<float> velDist(MIN_SPEED, MAX_SPEED);
        std::bernoulli_distribution velSign(0.5);

        // Initialize spheres with random positions and constant velocities
        for (int i = 0; i < numSpheres; i++) {
            Sphere sphere;
            sphere.position = { posDist(gen), posDist(gen), posDist(gen) };
            Vector3 sign = { 1 - 2 * velSign(gen), 1 - 2 * velSign(gen), 1 - 2 * velSign(gen) };
            sphere.velocity = { sign.x * velDist(gen), sign.y * velDist(gen), sign.z * velDist(gen) };

            spheres.push_back(sphere);
        }
    }

    inline void bounce(float& pos, float& vel, float bound) 
    {
        if (abs(pos) > bound) 
        {
            //vel = -vel;
            pos = pos > bound ? -bound : bound;
        }
    }

    void update() {
        for (auto& sphere : spheres) {
            // Update position with constant velocity
            sphere.position.x += sphere.velocity.x;
            sphere.position.y += sphere.velocity.y;
            sphere.position.z += sphere.velocity.z;

            bounce(sphere.position.x, sphere.velocity.x, CUBE_BOUND/2.0);
            bounce(sphere.position.y, sphere.velocity.y, CUBE_BOUND/2.0);
            bounce(sphere.position.z, sphere.velocity.z, CUBE_BOUND/2.0);
        }
    }

    void draw() {
        // Draw spheres
        for (const auto& sphere : spheres) {
            DrawSphere(sphere.position, sphereRadius, SPHERE_COLOR);
        }

        // Draw connections
        for (size_t i = 0; i < spheres.size(); i++) {
            for (size_t j = i + 1; j < spheres.size(); j++) {
                Vector3 p1 = spheres[i].position;
                Vector3 p2 = spheres[j].position;

                Vector3 d = { p2.x - p1.x, p2.y - p1.y , p2.z - p1.z };
                float distance = sqrt(d.x * d.x + d.y * d.y + d.z * d.z);

                if (distance <= connectionThreshold) {
                    if (distance <= 0.98f * connectionThreshold)
                        DrawLine3D(p1, p2, LINE_COLOR1);
                    else
                        DrawLine3D(p1, p2, LINE_COLOR2);
                }
            }
        }
    }
};

int main() {
    // Initialize window
    const int screenWidth = 1600;
    const int screenHeight = 1200;
    InitWindow(screenWidth, screenHeight, "3D Sphere Connections");

    // Initialize camera
    Camera3D camera = { 0 };
    camera.position = Vector3{ 0.0f, 0.0f, 0.8f * CUBE_BOUND };
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Create scene with 20 spheres and connection threshold of 5.0
    Scene scene(20, 3.0f);

    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose()) {
        scene.update();

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);
        scene.draw();
        // DrawGrid(20, 1.0f);  // Draw a reference grid
        EndMode3D();

        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}