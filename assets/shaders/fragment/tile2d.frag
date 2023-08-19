#version 300 es
precision highp float;

uniform sampler2D tilesetTexture;
smooth in vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
  fragColor = texture(tilesetTexture, fragTexCoord);
}
