#version 300 es
precision highp float;

smooth in vec3 fragPos;
flat in vec3 fragBasePos;
flat in vec3 fragLightColor;

out vec4 fragColor;

// Filled in by app settings and/or TileRenderer
layout (std140) uniform LightingParams
{
  vec3 ambientLight;
};

void main()
{
  float dist = 0.5f - length(fragPos - fragBasePos);
  fragColor = vec4(fragLightColor.rg, dist, dist);
  //fragColor = vec4(ambientLight, 0.8f);
}
