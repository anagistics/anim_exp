// main.cpp
#include "raylib.h"
#include <vector>
#include <random>
#include <cmath>

constexpr Vector3 minus(const Vector3& left, const Vector3& right)
{
    return { left.x - right.x, left.y - right.y, left.z - right.z };
}

constexpr Vector3 plus(const Vector3& left, const Vector3& right)
{
    return { left.x + right.x, left.y + right.y, left.z + right.z };
}

constexpr Vector3 scale(const Vector3& vec, float mult)
{
    return { mult * vec.x, mult * vec.y, mult * vec.z };
}

constexpr Color v2c(const Vector3& vec, unsigned char a = 255)
{
    return { static_cast<unsigned char>(vec.x), static_cast<unsigned char>(vec.y), static_cast<unsigned char>(vec.z), a };
}

constexpr Color colorScale(const Color& color, float mult, bool aconst = true)
{
    if (aconst)
        return { static_cast<unsigned char>(mult * color.r), static_cast<unsigned char>(mult * color.g)
        , static_cast<unsigned char>(mult * color.b), color.a };
    else
        return { static_cast<unsigned char>(mult * color.r), static_cast<unsigned char>(mult * color.g)
        , static_cast<unsigned char>(mult * color.b), static_cast<unsigned char>(mult * color.a) };
}

constexpr Color colorPlus(const Color& left, const Color& right)
{
    return { static_cast<unsigned char>(left.r + right.r), static_cast<unsigned char>(left.g + right.g)
        , static_cast<unsigned char>(left.b + right.b), static_cast<unsigned char>(left.a + right.a) };
}

constexpr Color colorMean(const Color& left, const Color& right)
{
    return colorPlus(colorScale(left, 0.5f), colorScale(right, 0.5f));
}


constexpr float CUBE_BOUND = 20.0f;
constexpr float CUBE_ZMAX = 10.0f;
constexpr float CUBE_ZMIN = -50.0f;
constexpr float SPHERE_RADIUS = 0.2f;
constexpr float MIN_SPEED = 0.01f;
constexpr float MAX_SPEED = 0.02f;
constexpr Color COLOR_LIGHT = BLUE;
constexpr Color COLOR_DARK = colorScale(DARKBLUE, 0.3f);
constexpr Color SPHERE_COLOR = COLOR_LIGHT;
constexpr Vector3 DARK = { COLOR_DARK.r, COLOR_DARK.g, COLOR_DARK.b };
constexpr Vector3 LIGHT = { COLOR_LIGHT.r, COLOR_LIGHT.g, COLOR_LIGHT.b };

// Structure to represent a sphere with position and velocity
struct Sphere {
    Vector3 position;
    Vector3 velocity;
    float radius = SPHERE_RADIUS;
    Color color = SPHERE_COLOR;
    bool connectable = true;
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
    }

    void update() {
        static constexpr float rim_end = 0.8f * CUBE_ZMIN;
        static constexpr float rim_start = 0.8f * rim_end;
        static constexpr float rim_width = rim_end - rim_start;
        static constexpr Vector3 color_range = minus(LIGHT, DARK);

        for (auto& sphere : spheres) {
            // Update position with constant velocity
            sphere.position = plus(sphere.position, sphere.velocity);
            sphere.connectable = true;
            if (sphere.position.z < 0)
            {
                if (sphere.position.z < rim_end)
                {
                    sphere.color = COLOR_DARK;
                    sphere.connectable = false;
                }
                else if (sphere.position.z < rim_start)
                {
                    const float sf = (rim_end - sphere.position.z) / rim_width;
                    sphere.color = v2c(plus(DARK, scale(color_range, sf)));
                }
                else
                    sphere.color = COLOR_LIGHT;
            }
            else 
                sphere.color = COLOR_LIGHT;

            bounce(sphere.position.x, sphere.velocity.x, CUBE_BOUND/2.0);
            bounce(sphere.position.y, sphere.velocity.y, CUBE_BOUND/2.0);
            zbounce(sphere, CUBE_ZMIN, CUBE_ZMAX);

            sphere.color.r = (sphere.position.z - CUBE_ZMIN) / (CUBE_ZMAX - CUBE_ZMIN) * 255;
        }
    }

    void draw() {
        // Draw spheres
        for (const auto& sphere : spheres) {
            DrawSphere(sphere.position, sphere.radius, sphere.color);
        }

        // Draw connections
        for (size_t i = 0; i < spheres.size(); i++) {
            if (!spheres[i].connectable)
                continue;
            for (size_t j = i + 1; j < spheres.size(); j++) {
                if (!spheres[j].connectable)
                    continue;
                Vector3 p1 = spheres[i].position;
                Vector3 p2 = spheres[j].position;

                Vector3 d = { p2.x - p1.x, p2.y - p1.y , p2.z - p1.z };
                float distanceSqr = d.x * d.x + d.y * d.y + d.z * d.z;

                if (distanceSqr <= connectionThresholdSqr) {
                    if (distanceSqr <= 0.9f * connectionThresholdSqr)
                        DrawLine3D(p1, p2, colorMean(spheres[i].color, spheres[j].color));
                    else
                        DrawLine3D(p1, p2, v2c(DARK));
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
    camera.fovy = 30.0f; //45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Create scene with 20 spheres and connection threshold of 5.0
    Scene scene(30, 5.0f);

    SetTargetFPS(60);
    bool showGrid = false;

    // Main game loop
    while (!WindowShouldClose()) {
        scene.update();

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);
        scene.draw();
        if (showGrid)
            DrawGrid(20, 1.0f);  // Draw a reference grid   
        EndMode3D();

        if (IsKeyPressed(KEY_G))
            showGrid = !showGrid;

        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}