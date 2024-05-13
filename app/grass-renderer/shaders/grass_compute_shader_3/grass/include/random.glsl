#version 450

float random(vec2 seed)
{
    return fract(
        sin(
            dot(seed, vec2(12.9898, 78.233))
        ) * 43758.5453
    );
}

float valueNoise(vec2 uv)
{
    vec2 i = floor(uv);
    vec2 f = fract(uv);

    f = f * f * (3.0 - 2.0 * f);

    uv = abs(fract(uv) - 0.5);
    vec2 c0 = i + vec2(0.0, 0.0);
    vec2 c1 = i + vec2(1.0, 0.0);
    vec2 c2 = i + vec2(0.0, 1.0);
    vec2 c3 = i + vec2(1.0, 1.0);
    float r0 = random(c0);
    float r1 = random(c1);
    float r2 = random(c2);
    float r3 = random(c3);

    float bottomOfGrid = mix(r0, r1, f.x);
    float topOfGrid = mix(r2, r3, f.x);
    float t = mix(bottomOfGrid, topOfGrid, f.y);

    return t;
}

void simpleNoise(vec2 uv, float scale, out float noise)
{
    float t = 0.0;

    float freq = pow(2.0, float(0));
    float amp = pow(0.5, float(3 - 0));
    t += valueNoise(vec2(uv.x * scale / freq, uv.y * scale / freq)) * amp;

    freq = pow(2.0, float(1));
    amp = pow(0.5, float(3 - 1));
    t += valueNoise(vec2(uv.x * scale / freq, uv.y * scale / freq)) * amp;

    freq = pow(2.0, float(2));
    amp = pow(0.5, float(3 - 2));
    t += valueNoise(vec2(uv.x * scale / freq, uv.y * scale / freq)) * amp;

    noise = t;
}

float randomWeighted(vec4 components, vec4 weights, vec2 seed)
{
    vec4 sum = vec4(0.0);

    // Sum of weights MUST be 1.0
    sum.rgba += components.r;
    sum.gba += components.g;
    sum.ba += components.b;
    sum.a += components.a;

    float random = random(seed);
    sum = step(random, sum);

    // Make sure to keep only the first 1.0 in rgba
    sum.gba -= sum.rgb;

    return dot(sum, components);
}

