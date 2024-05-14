#version 450


vec3 quadraticBezierCurve(vec3 p0, vec3 p1, vec3 p2, float t)
{
    float a = pow(1 - t, 2);
    float b = 2 * (1 - t) * t;
    float c = pow(t, 2);

    return a * p0 + b * p1 + c * p2;
}

vec3 quadraticBezierCurveTangent(vec3 p0, vec3 p1, vec3 p2, float t)
{
    float a = 2 * (1 - t);
    float b = 2 * t;

    return a * (p1 - p0) + b * (p2 - p1);
}

vec3 cubicBezierCurve(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t)
{
    float a = pow(1 - t, 3);
    float b = 3 * pow(1 - t, 2) * t;
    float c = 3 * (1 - t) * pow(t, 2);
    float d = pow(t, 3);

    return a * p0 + b * p1 + c * p2 + d * p3;
}

vec3 quadraticBezierCurveTangent(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t)
{
    float a = 3 * pow(1 - t, 2);
    float b = 6 * (1 - t) * t;
    float c = 3 * pow(t, 2);

    return a * (p1 - p0) + b * (p2 - p1) + c * (p3 - p2);
}
