#version 450

uniform vec3 CameraPositionWorldSpace;

#define FRUSTUM_CULLING_SMOOTH 0

#if FRUSTUM_CULLING_SMOOTH == 0

bool FrustumCulling(vec4 clipPosition)
{
    return all(
        lessThan(clipPosition, vec4(clipPosition.w))
    ) && all(greaterThan(clipPosition, vec4(-clipPosition.w)));
}

#else

uniform float DistanceCullStart;
uniform float DistanceCullEnd;
uniform float DistanceCullMinimumGrassAmount;

uniform float FrustumCullEdgeOffset;
uniform float FrustumCullNearOffset;

uint DistanceCulling(vec3 worldPosition, float hash, vec3 posToCam) {
    float d = distance(worldPosition, CameraPositionWorldSpace);
    float distanceSmoothStep = 1 - smoothstep(DistanceCullStart, DistanceCullEnd, d);
    distanceSmoothStep = (distanceSmoothStep * (1 - DistanceCullMinimumGrassAmount)) + DistanceCullMinimumGrassAmount;
    return hash > 1 - distanceSmoothStep;
}

bool FrustumCulling(vec4 clipPosition)
{
    return all(
        lessThan(clipPosition, vec4(clipPosition.w) + vec4(FrustumCullEdgeOffset, FrustumCullNearOffset, 0, 0))
    ) && all(greaterThan(clipPosition, vec4(-clipPosition.w) + vec4(-FrustumCullEdgeOffset, -FrustumCullNearOffset, 0, 0)));
}

#endif



