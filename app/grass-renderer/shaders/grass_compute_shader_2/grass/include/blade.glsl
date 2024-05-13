#version 450

// TODO Not so sure about the alignment
struct GrassBlade { // NOLINT(*-pro-type-member-init)
    vec3 position;
    float rotAngle;
    vec3 surfaceNorm;
    float hash;
    vec3 color;
    float height;
    vec2 facing;
    float width;
    float tilt;
    float bend;
    float windForce;
    float sideBend;
    float clumpColorDistanceFade;
};

struct Blade // NOLINT(*-pro-type-member-init)
{
    vec3 position;
    float windOffset;
};
