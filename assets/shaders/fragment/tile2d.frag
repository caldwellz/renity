#version 300 es
precision highp float;

const uint MAX_MAP_LIGHTS = 15u;
uniform sampler2D tilesetTexture;
flat in vec3 lightColors[MAX_MAP_LIGHTS];
smooth in float lightDistances[MAX_MAP_LIGHTS];
smooth in vec2 fragTexCoord;
out vec4 fragColor;

// Filled in by app settings and/or TileRenderer
layout (std140) uniform LightingParams
{
  vec3 ambientLight;
};

struct LightConstants {
  float constantTerm;
  float linearTerm;
  float quadraticTerm;
};
// See https://wiki.ogre3d.org/Light+Attenuation+Shortcut
const LightConstants light = LightConstants(1.0f, 4.5f, 75.0f);

void main()
{
  vec4 tileColor = texture(tilesetTexture, fragTexCoord);
  vec3 mixedLightColor = vec3(0.0f, 0.0f, 0.0f);
  for (uint idx = 0u; idx < MAX_MAP_LIGHTS; ++idx) {
    vec3 lightColor = lightColors[idx];
    // Have we reached the end flag?
    if (length(lightColor) < 0.001) {
      break;
    }

    float lightDist = lightDistances[idx];
    float attenuation = 1.0f / (light.constantTerm + light.linearTerm * lightDist + light.quadraticTerm * (lightDist * lightDist));
    mixedLightColor += lightColor * attenuation;
  }
  fragColor = vec4(tileColor.rgb * (ambientLight + mixedLightColor) * 2.0f, tileColor.a);
}
