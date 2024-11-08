// main.cpp
#include "raylib.h"
#include <vector>
#include <random>
#include <cmath>

constexpr float CUBE_BOUND = 10.0f;
constexpr float CUBE_ZMAX = 10.0f;
constexpr float CUBE_ZMIN = -10.0f;
constexpr float SPHERE_RADIUS = 0.2f;
constexpr float MIN_SPEED = 0.01f;
constexpr float MAX_SPEED = 0.02f;
constexpr Color COLOR_LIGHT = BLUE;
constexpr Color COLOR_DARK = DARKBLUE;
constexpr Color SPHERE_COLOR = COLOR_LIGHT;

// Structure to represent a sphere with position and velocity
struct Sphere {
    Vector3 position;
    Vector3 velocity;
    float radius = SPHERE_RADIUS;
    Color color = SPHERE_COLOR;
};

class Scene {
private:
    std::vector<Sphere> spheres;
    float connectionThresholdSqr;
   
public:
    Scene(int numSpheres, float threshold)
        : connectionThresholdSqr(threshold * threshold)
    {
        // Random number generation
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posDist(-CUBE_BOUND / 2.0, CUBE_BOUND / 2.0);
        std::uniform_real_distribution<float> zposDist(CUBE_ZMIN, CUBE_ZMAX);
        std::uniform_real_distribution<float> velDist(MIN_SPEED, MAX_SPEED);
        std::uniform_real_distribution<float> zvelDist(2.0f * MIN_SPEED, 4.0f * MAX_SPEED);
        std::bernoulli_distribution velSign(0.5);
        std::bernoulli_distribution zvelSign(0.8);

        // Initialize spheres with random positions and constant velocities
        for (int i = 0; i < numSpheres; i++) {
            Sphere sphere;
            sphere.position = { posDist(gen), posDist(gen), zposDist(gen) };
            Vector3 sign = { 1 - 2 * velSign(gen), 1 - 2 * velSign(gen), 1 - 2 * zvelSign(gen) };
            sphere.velocity = { sign.x * velDist(gen), sign.y * velDist(gen), sign.z * zvelDist(gen) };

            spheres.push_back(sphere);
        }
    }

    inline void bounce(float& pos, float& vel, float bound) const
    {
        if (abs(pos) > bound)
            pos = pos > bound ? -bound : bound;
    }

    inline void zbounce(Sphere& sphere, float boundMin, float boundMax) const
    {
        if (sphere.position.z > boundMax)
        {
            sphere.position.z = boundMin;
        }
        else if (sphere.position.z < boundMin)
        {
            sphere.position.z = boundMax;
        } 
        sphere.color.r = 255u * (sphere.position.z - CUBE_ZMIN) / (CUBE_ZMAX - CUBE_ZMIN);
    }

    void update() {
        for (auto& sphere : spheres) {
            // Update position with constant velocity
            sphere.position.x += sphere.velocity.x;
            sphere.position.y += sphere.velocity.y;
            sphere.position.z += sphere.velocity.z;

            bounce(sphere.position.x, sphere.velocity.x, CUBE_BOUND/2.0);
            bounce(sphere.position.y, sphere.velocity.y, CUBE_BOUND/2.0);
            zbounce(sphere, CUBE_ZMIN, CUBE_ZMAX);
        }
    }

    void draw() {
        // Draw spheres
        for (const auto& sphere : spheres) {
            DrawSphere(sphere.position, sphere.radius, sphere.color);
        }

        // Draw connections
        for (size_t i = 0; i < spheres.size(); i++) {
            for (size_t j = i + 1; j < spheres.size(); j++) {
                Vector3 p1 = spheres[i].position;
                Vector3 p2 = spheres[j].position;

                Vector3 d = { p2.x - p1.x, p2.y - p1.y , p2.z - p1.z };
                float distanceSqr = d.x * d.x + d.y * d.y + d.z * d.z;

                if (distanceSqr <= connectionThresholdSqr) {
                    if (distanceSqr <= 0.9f * connectionThresholdSqr)
                        DrawLine3D(p1, p2, COLOR_LIGHT);
                    else
                        DrawLine3D(p1, p2, COLOR_DARK);
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
    camera.position = Vector3{ -1.0f, 1.0f, 0.9f * CUBE_ZMAX };
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