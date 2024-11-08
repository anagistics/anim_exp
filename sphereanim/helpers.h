#ifndef __HELPERS__
#define __HELPERS__

namespace {
    // Helping coordinate system
    Vector3 origin = { -3.0f, -3.0f, 3.0f };  // Ursprung des Koordinatensystems
    float axisLength = 1.0f;  // Länge der Achsen
}

void DrawCoordinateSystem(Vector3 origin, float axisLength)
{
    // X-Achse (rot)
    DrawLine3D(origin, Vector3{ origin.x + axisLength, origin.y, origin.z }, RED);

    // Y-Achse (grün)
    DrawLine3D(origin, Vector3{ origin.x, origin.y + axisLength, origin.z }, GREEN);

    // Z-Achse (blau)
    DrawLine3D(origin, Vector3{ origin.x, origin.y, origin.z + axisLength }, BLUE);
}

void DrawTextIn3D(const char* text, Vector3 position, float fontSize, Color color, Camera camera)
{
    // Berechnen Sie die 2D-Bildschirmposition für den 3D-Punkt
    Vector2 screenPosition = GetWorldToScreen(position, camera);

    // Zeichnen Sie den Text an der berechneten 2D-Position
    DrawText(text, (int)screenPosition.x, (int)screenPosition.y, (int)fontSize, color);
}

void DrawAxisLabels(Vector3 origin, float axisLength, Camera camera)
{
    Vector3 endX = { origin.x + axisLength, origin.y, origin.z };
    Vector3 endY = { origin.x, origin.y + axisLength, origin.z };
    Vector3 endZ = { origin.x, origin.y, origin.z + axisLength };

    DrawTextIn3D("X", endX, 10.0f, RED, camera);
    DrawTextIn3D("Y", endY, 10.0f, GREEN, camera);
    DrawTextIn3D("Z", endZ, 10.0f, BLUE, camera);
}

class Color;

bool operator==(const Color& left, const Color& right)
{
    return left.r == right.r && left.g == right.g && left.b == right.b && left.a == right.a;
}

bool operator<(const Color& left, const Color& right)
{
    return left.r < right.r && left.g < right.g && left.b < right.b && left.a <= right.a;
}


Color darken(const Color& col, float rate = 0.9f)
{
    Color darker;;
    darker.r = static_cast<unsigned char>(col.r * rate);
    darker.g = static_cast<unsigned char>(col.g * rate);
    darker.b = static_cast<unsigned char>(col.b * rate);
    return darker;
}


#endif
