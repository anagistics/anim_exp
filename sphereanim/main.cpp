// main.cpp
#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <random>
#include <cmath>
#include <sstream>
#include <string>

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

inline float Norm(Vector3 a)
{
    return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

inline float Dist(Vector3 a, Vector3 b)
{
    return Norm({ a.x - b.x, a.y - b.y, a.z - b.y });
}

constexpr float CUBE_BOUND = 20.0f;
constexpr float CUBE_ZMAX = 10.0f;
constexpr float CUBE_ZMIN = -50.0f;
constexpr float SPHERE_RADIUS = 0.5f;
constexpr float MIN_SPEED = 0.01f;
constexpr float MAX_SPEED = 0.02f;
constexpr int WIRE_COUNT = 4;
constexpr Vector3 MIN_POS = { -CUBE_BOUND, -CUBE_BOUND, CUBE_ZMIN };
constexpr Vector3 MAX_POS = { CUBE_BOUND, CUBE_BOUND, CUBE_ZMAX };
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

    template<int arg = 2>
    void fadeInOut(Sphere& sphere, float rimStart, float rimEnd) const
    {
        static constexpr Vector3 color_range = minus(LIGHT, DARK);
        const float rim_width = rimEnd - rimStart;
        float* coord = nullptr;
        if constexpr (arg == 0 || arg == 1)
        {
            if (arg == 0)
                coord = &sphere.position.x;
            else
                coord = &sphere.position.y;
            float pos = abs(*coord);
            if (pos > rimEnd)
            {
                sphere.color = COLOR_DARK;
                sphere.connectable = false;
            }
            else if (pos > rimStart)
            {
                const float sf = (rimEnd - pos) / rim_width;
                sphere.color = v2c(plus(DARK, scale(color_range, sf)));
            }
            else
                sphere.color = COLOR_LIGHT;
        }
        else
        {
            coord = &sphere.position.z;
            if (*coord < 0)
            {
                if (*coord < rimEnd)
                {
                    sphere.color = COLOR_DARK;
                    sphere.connectable = false;
                }
                else if (*coord < rimStart)
                {
                    const float sf = (rimEnd - *coord) / rim_width;
                    sphere.color = v2c(plus(DARK, scale(color_range, sf)));
                }
                else
                    sphere.color = COLOR_LIGHT;
            }
            else
                sphere.color = COLOR_LIGHT;
        }
    }

    void update() {
        static constexpr float rim_end = 0.8f * CUBE_ZMIN;
        static constexpr float rim_start = 0.8f * rim_end;
        static constexpr float rim_endxy = 0.8f * CUBE_BOUND;
        static constexpr float rim_startxy = 0.8f * rim_endxy;

        for (auto& sphere : spheres) {
            // Update position with constant velocity
            sphere.position = plus(sphere.position, sphere.velocity);
            sphere.connectable = true;
            /*fadeInOut<0>(sphere, rim_startxy, rim_endxy);
            fadeInOut<1>(sphere, rim_startxy, rim_endxy);
            fadeInOut<2>(sphere, rim_start, rim_end);*/

            bounce(sphere.position.x, sphere.velocity.x, CUBE_BOUND/2.0);
            bounce(sphere.position.y, sphere.velocity.y, CUBE_BOUND/2.0);
            zbounce(sphere, CUBE_ZMIN, CUBE_ZMAX);

            sphere.color.r = (sphere.position.z - CUBE_ZMIN) / (CUBE_ZMAX - CUBE_ZMIN) * 255;
        }
    }

    void draw() {
        // Draw spheres
        for (const auto& sphere : spheres) {
            DrawSphereWires(sphere.position, sphere.radius, WIRE_COUNT, WIRE_COUNT, sphere.color);
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

        Vector3 size_dummy = Vector3Subtract(MAX_POS, MIN_POS);
        Color wall = BLACK;
        wall.a = 200;
        Vector3 z_offset{ 0.0f, 0.0f, 29.5f };
        BeginBlendMode(BLEND_ALPHA);
        DrawCube(Vector3Add(Vector3Add(MAX_POS, Vector3Scale(size_dummy, -0.5f)), z_offset), size_dummy.x, size_dummy.y, 0.25f, wall);
        EndBlendMode();

    }
};

enum class XYDirection : size_t { X, Y };
template<XYDirection dir>
void RotateByAxis(Camera& camera, float rad)
{
    float angle = PI * rad / 180.0;
    Vector3 rotAxis;
    if constexpr (dir == XYDirection::Y)
        rotAxis = Vector3{ 1.0f, 0.0f, 0.0f };
    else
        rotAxis = Vector3{ 0.0f, 1.0f, 0.0f };
    Vector3 rotatedAxis = Vector3RotateByAxisAngle(Vector3Subtract(camera.target, camera.position), rotAxis, angle);
    camera.position = Vector3Subtract(camera.target, rotatedAxis);
}

int main() {
    // Initialize window
    const int screenWidth = 1600;
    const int screenHeight = 1200;
    InitWindow(screenWidth, screenHeight, "3D Sphere Connections");

    // Initialize camera
    Camera3D camera;
    Vector3 defaultCameraPos = Vector3{ -2.0f, 3.0f, -5.0f * CUBE_ZMAX };
    Vector3 target = Vector3{ 0.0f, 0.0f, 0.0f };

    camera.position = defaultCameraPos;
    camera.target = target;
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 30.0f; //45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Create scene with 30 spheres and connection threshold of 5.0
    Scene scene(30, 7.0f);

    SetTargetFPS(60);
    bool showGrid = false;
    bool showBox = false;

    // Main game loop
    while (!WindowShouldClose()) {
        scene.update();

        BeginDrawing();
        ClearBackground(BLACK);
        // UpdateCamera(&camera, CAMERA_CUSTOM);
        BeginMode3D(camera);
        scene.draw();
        if (showGrid)
        {
            DrawGrid(200, 5.0f);  // Draw a reference grid   
            DrawSphere(target, 2, ORANGE);
        }
        if (showBox)
        {
            DrawBoundingBox({ MIN_POS, MAX_POS }, YELLOW);
            DrawSphere(MIN_POS, 2, GREEN);
            DrawSphere(MAX_POS, 2, RED);
        }
        EndMode3D();

        if (IsKeyPressedRepeat(KEY_LEFT) || IsKeyPressedRepeat(KEY_RIGHT) || IsKeyPressedRepeat(KEY_DOWN) || IsKeyPressedRepeat(KEY_UP))
        {
            constexpr float angleIncr = 0.1f;
            if (IsKeyPressedRepeat(KEY_LEFT))
                RotateByAxis<XYDirection::X>(camera, -angleIncr);
            if (IsKeyPressedRepeat(KEY_RIGHT))
                RotateByAxis<XYDirection::X>(camera, angleIncr);
            if (IsKeyPressedRepeat(KEY_DOWN))
                RotateByAxis<XYDirection::Y>(camera, -angleIncr);
            if (IsKeyPressedRepeat(KEY_UP))
                RotateByAxis<XYDirection::Y>(camera, angleIncr);
        }
        else
        {
            if (IsKeyPressedRepeat(KEY_PAGE_DOWN))
                camera.position.z -= 1;
            else if (IsKeyPressedRepeat(KEY_PAGE_UP))
                camera.position.z += 1;
            else if (IsKeyReleased(KEY_Y))  // this is z on a German keyboard!
                camera.position = defaultCameraPos;
        }

        if (IsKeyPressed(KEY_G))
            showGrid = !showGrid;
        else if (IsKeyPressed(KEY_B))
            showBox = !showBox;

        DrawFPS(10, 10);
        std::ostringstream out;
        out << "X:" << camera.position.x << " Y:" << camera.position.y << " Z:" << camera.position.z;
        DrawText(out.str().c_str(), 20, 1150, 24, YELLOW);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}