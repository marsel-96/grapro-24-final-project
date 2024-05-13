#version 450

struct GrassBlade
{
    // IDK if vec3 + float gets packed properly so for now vec4 is the the safest bet
    vec3 position;
    float windForce;
    float hash;
    uint grassType;
    float clumpColor;
    float height;
    float width;
    float tilt;
    float bend;
    float sideCurve;
    vec2 facing;
    vec2 clumpFacing;
};
