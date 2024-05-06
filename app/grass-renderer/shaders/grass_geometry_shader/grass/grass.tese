#version 410 core

layout (triangles, equal_spacing, cw, point_mode) in;

in VS_OUT {
    vec3 position;
    vec3 normal;
    vec3 tangent;
} vs_out[];

out VS_OUT {
    vec3 position;
    vec3 normal;
    vec3 tangent;
} ts_out;

#define INTERPOLATE(fieldName) ts_out.fieldName = \
    vs_out[0].fieldName * gl_TessCoord.x + \
    vs_out[1].fieldName * gl_TessCoord.y + \
    vs_out[2].fieldName * gl_TessCoord.z

uniform mat4 ViewProjMatrix;

void main()
{
    INTERPOLATE(position);
    INTERPOLATE(normal);
    INTERPOLATE(tangent);

    gl_Position = ViewProjMatrix * vec4(ts_out.position, 1.0);


}