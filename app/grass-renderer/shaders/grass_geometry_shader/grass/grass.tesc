#version 410 core

layout (vertices = 3) out;

uniform float TessellationUniform;

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
        gl_TessLevelOuter[0] = TessellationUniform;
        gl_TessLevelOuter[1] = TessellationUniform;
        gl_TessLevelOuter[2] = TessellationUniform;
        gl_TessLevelOuter[3] = TessellationUniform;

        gl_TessLevelInner[0] = TessellationUniform;
        gl_TessLevelInner[1] = TessellationUniform;
    }
}