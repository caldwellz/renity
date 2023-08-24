#version 300 es
precision highp float;

layout (location = 0) in vec3 vertCoords;
layout (location = 1) in vec2 vertUv;
layout (location = 2) in vec3 tilePos;
layout (location = 3) in vec3 tileTuv;

// Max number of lights allowed by the varying variables threshold
const uint MAX_MAP_LIGHTS = 15u;
const uint MAX_LIGHT_DETAILS = MAX_MAP_LIGHTS * 2u;
flat out vec3 lightColors[MAX_MAP_LIGHTS];
smooth out float lightDistances[MAX_MAP_LIGHTS];
smooth out vec2 fragTexCoord;

// Filled in by app settings and/or TileRenderer
layout (std140) uniform ViewParams
{
  vec2 viewSize;
  float scale;
};

// Filled in by Tilesets
layout (std140) uniform TilesetDetails
{
  vec2 tileSize;
  vec2 tilesetSize;
};

// Filled in by Tilemaps
layout (std140) uniform MapDetails
{
  vec2 mapPosition;
  float mapInverseSizeY;
  float mapDepthRange;
  vec4 lightDetails[MAX_LIGHT_DETAILS];
};

vec3 getNormalizedPos(vec3 vertPos, vec3 instancePos) {
  vec2 pixelScale = (2.0f * scale) / viewSize;
  vec3 normalizedVertPos = vec3((vertPos.xy + 1.0f) / 2.0f * pixelScale * tileSize, vertPos.z);
  vec3 normalizedTilePos = vec3(instancePos.xy * pixelScale, instancePos.z / mapDepthRange);
  vec3 normalizedMapPos = vec3(0.0f, mapInverseSizeY * pixelScale.y, 0.0f);
  vec3 normalizedCameraPos = vec3(mapPosition * pixelScale, 0.0f);
  return (
    normalizedVertPos
  + normalizedTilePos
  + normalizedMapPos
  + normalizedCameraPos);
}

void main()
{
  vec3 actualPos = getNormalizedPos(vertCoords, tilePos);
  gl_Position = vec4(actualPos, 1.0f);
  vec2 tilesetScale = 1.0f / tilesetSize;
  fragTexCoord = (tileTuv.xy * tilesetScale) + (vertUv * tilesetScale * tileSize);

  // Convert light source positions into distances and pass them along
  float lightAspect = (viewSize.x / viewSize.y);
  uint light;
  for (light = 0u; light < MAX_MAP_LIGHTS; ++light) {
    uint idx = light * 2u;
    vec4 lightColor = lightDetails[idx];
    // An early end of the incoming list will be zeroed out
    if (lightColor.a < 0.001f) {
      // Flag the end of the outputs by zeroing out the color
      lightColors[light] = vec3(0.0f, 0.0f, 0.0f);
      break;
    }
    lightColors[light] = lightColor.rgb;

    // Use just a single vertex (the bottom-left) for smooth rather than blocky lighting
    // vec3 lightPos = getNormalizedPos(vertCoords, lightDetails[++idx].xyz);
    vec3 lightPos = getNormalizedPos(vec3(-1.0f, -1.0f, 0.0f), lightDetails[++idx].xyz);
    float lightDist = distance(vec2(actualPos.x * lightAspect, actualPos.y), vec2(lightPos.x * lightAspect, lightPos.y));
    lightDistances[light] = lightDist / scale;
  }
}
