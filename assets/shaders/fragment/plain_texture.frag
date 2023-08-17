#version 300 es
precision mediump float;

uniform sampler2D myTexture;
in vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
  // fragColor = vec4(fragTexCoord.x, 0.0, fragTexCoord.y, 1.0);
  fragColor = texture(myTexture, fragTexCoord);
  // Enable early depth testing discards by not manually setting the depth anywhere
  // gl_FragDepth = gl_FragCoord.z;
}
