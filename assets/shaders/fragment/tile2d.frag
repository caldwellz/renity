#version 300 es
precision highp float;

uniform sampler2D tilesetTexture;
smooth in vec2 fragTexCoord;
out vec4 fragColor;

// Filled in by app settings and/or TileRenderer
layout (std140) uniform LightingParams
{
  vec3 ambientLight;
};

void main()
{
  vec4 tileColor = texture(tilesetTexture, fragTexCoord);
  fragColor = vec4(tileColor.rgb * ambientLight * 2.0f, tileColor.a);
}
