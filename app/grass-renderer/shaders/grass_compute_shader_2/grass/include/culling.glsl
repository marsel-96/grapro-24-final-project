#version 450

uniform float DistanceCullStart = 0.0;
uniform float DistanceCullEnd = 0.0;
uniform float DistanceCullThreshold = 0.0;
uniform float DistanceCullMinimumGrassAmount = 0.0;

uniform vec3 CameraPositionWorldSpace;

uniform float FrustumCullEdgeOffset = 0.0;
uniform float FrustumCullNearOffset = 0.0;

uniform sampler2D DepthTexture;

// TODO : Uniform should be in upper level or passed as argument

uint DistanceCulling(vec3 worldPosition) {
    // Get the world position of current grass instance
    // Get the world position of main camera
    // Calculate the distance between two positions
    // Cull if distance exeeds the threshold

    float d = distance(worldPosition, CameraPositionWorldSpace);

    if (d <= DistanceCullThreshold)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint DistanceCullingSmooth(vec3 worldPosition, float hash, vec3 posToCam) {
    //float viewAngleModulation = pow(saturate(dot(norm, posToCam)  * _DistanceCullC), _Test);
    //float viewAngleCull =  hash > (1-viewAngleModulation) ? 1 : -1;

    //Distance culling
    float d = distance(worldPosition, CameraPositionWorldSpace);
    float distanceSmoothStep = 1 - smoothstep(DistanceCullStart, DistanceCullEnd, d);
    distanceSmoothStep = (distanceSmoothStep * (1 - DistanceCullMinimumGrassAmount)) + DistanceCullMinimumGrassAmount;

    //distanceSmoothStep *= viewAngleModulation;

    //float combined = viewAngleCull * _DistanceCullM +(1-distanceSmoothStep);

    return hash > 1 - distanceSmoothStep ? 1 : 0;


    //return saturate(viewAngleCull * _DistanceCullM + distanceCull);
    //return distanceCulling > _CullBoundary ? 1 : 0;
}



// Position is in clip space
uint FrustumCulling(vec4 position) {
    return (
    position.z > position.w
    || position.z < -position.w
    || position.x < -position.w + FrustumCullEdgeOffset
    || position.x > position.w - FrustumCullEdgeOffset
    || position.y < -position.w + FrustumCullNearOffset
    || position.y > position.w)
    ? 0 : 1;
}


// Position is in clip space
uint FrustumCulling(vec4 position) {
    // The size of clipping space is about [-clippingPos.w, clippingPos.w]
    // Only need to check if x y z values are inside the range
    bool isOutsideFarNearPlane = position.z > position.w || position.z < -position.w;
    bool isOutsideLeftRightPlane = position.x < -position.w + FrustumCullEdgeOffset || position.x > position.w - FrustumCullEdgeOffset;
    bool isOutsideBottomTopPlane = position.y < -position.w + FrustumCullNearOffset || position.y > position.w;

    if (isOutsideFarNearPlane || isOutsideLeftRightPlane || isOutsideBottomTopPlane)
    {
        return 0; // The point is outside the frustum
    }
    else
    {
        return 1; // The point is inside the frustum
    }
}

uniform float DepthTextureSize = 0.0;

// Position is in clip space
uint OcclusionCulling(vec4 position) {
    // Transfer grass position from clipping to NDC
    vec3 positionNDC = position.xyz / position.w;

    // Transfer to uv coordinate
    vec2 positionUV = vec2(positionNDC.x, positionNDC.y) * 0.5f + 0.5f;

    // Use maximum mipmap level
    uint mipmapLevel = log2(DepthTextureSize) - 1;
    uint size = DepthTextureSize / (1 << mipmapLevel);

    // Calculate the pixel position
    uvec2 pixelPosition = uvec2(
        clamp(positionUV.x * size, 0, size - 1),
        clamp(positionUV.y * size, 0, size - 1)
    );

    // Sample the depth in depth texture
    float depthInTexture = textureLod(DepthTexture, positionUV, 0).r;

    float depth = positionNDC.z;

    if (depth > depthInTexture)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
