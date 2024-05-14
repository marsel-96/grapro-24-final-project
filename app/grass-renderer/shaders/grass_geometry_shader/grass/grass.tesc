#version 410 core

layout (vertices = 3) out;

uniform vec4 TessellationLevelOuter;
uniform vec2 TessellationLevelInner;

in VS_OUT {
    vec3 position;
    vec3 normal;
    vec3 tangent;
} ts_in[];

out VS_OUT {
    vec3 position;
    vec3 normal;
    vec3 tangent;
} ts_out[];

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    ts_out[gl_InvocationID].position = ts_in[gl_InvocationID].position;
    ts_out[gl_InvocationID].tangent = ts_in[gl_InvocationID].tangent;
    ts_out[gl_InvocationID].normal = ts_in[gl_InvocationID].normal;

    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = TessellationLevelOuter.x;
        gl_TessLevelOuter[1] = TessellationLevelOuter.y;
        gl_TessLevelOuter[2] = TessellationLevelOuter.z;
        gl_TessLevelOuter[3] = TessellationLevelOuter.w;

        gl_TessLevelInner[0] = TessellationLevelInner.x;
        gl_TessLevelInner[1] = TessellationLevelInner.y;
    }
}