#version 300 es
precision highp float;

uniform sampler2D myTexture;
in vec2 fragTexCoord;
out vec4 fragColor;

layout (std140) uniform TilesetDetails
{
  uvec2 tileCounts;
  uint edgeSize;
};

void main()
{
  // ES3 doesn't have GL_POINT_SPRITE_COORD_ORIGIN, so the Y has to be flipped manually
  vec2 realPointCoord = vec2(gl_PointCoord.x, 1.0f - gl_PointCoord.y);

  vec2 texIncrements = vec2(1.0f / float(tileCounts.x), 1.0f / float(tileCounts.y));

  fragColor = vec4(fragTexCoord.xy + 0.1f, 0.1f, 1.0);
  //fragColor = texture(myTexture, fragTexCoord + (realPointCoord * texIncrements));
  //fragColor = vec4(gl_PointCoord, 0.3, 0.9);

  // Optional, emulate GL_ALPHA_TEST to use transparent images with
    // point sprites without worrying about z-order.
    // see: http://stackoverflow.com/a/5985195/806988
    if(fragColor.a == 0.0){
        discard;
    }
}
