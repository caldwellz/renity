#version 300 es
precision highp float;

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 vertUv;
layout (location = 2) in vec3 tilePos;
layout (location = 3) in vec3 tileTuv;

smooth out vec3 fragPos;
flat out vec3 fragBasePos;
flat out vec3 fragLightColor;

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
  vec2 mapSize;
  float mapDepthRange;
};

void main()
{
  vec2 pixelScale = (2.0f * scale) / viewSize;
  vec3 normalizedVertPos = vec3((vertPos.xy + 1.0f) / 2.0f * pixelScale * tileSize, vertPos.z);
  vec3 normalizedTilePos = vec3(tilePos.xy * pixelScale, tilePos.z / mapDepthRange);
  vec3 normalizedMapPos = vec3(0.0f, -mapSize.y * pixelScale.y, 0.0f);
  vec3 normalizedCameraPos = vec3(mapPosition * pixelScale, -0.5f);
  fragBasePos = vec3(
      normalizedVertPos * 4.0f
    + normalizedTilePos
    + normalizedMapPos
    + normalizedCameraPos);
  fragPos = fragBasePos;
  gl_Position = vec4(fragPos, 1.0f);

  fragLightColor = tileTuv / 255.0f;
}
