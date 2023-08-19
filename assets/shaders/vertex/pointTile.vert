#version 300 es
precision highp float;

layout (location = 0) in vec3 pointPos;
layout (location = 1) in uvec2 tileIndex;

out vec2 fragTexCoord;

// This gets filled in by the app settings
layout (std140) uniform ViewParams
{
  float aspect;
  float scale;
};

// This gets filled in by Tileset resources
layout (std140) uniform TilesetDetails
{
  uvec2 tileCounts;
  uint edgeSize;
};

void main()
{
  float aspectY = 1.0f / aspect;
  gl_PointSize = float(edgeSize) * scale;
  /* gl_Position = vec4((pointPos.x  * scale) / (aspect * 2.0f), pointPos.y * (scale / 2.0f), pointPos.z, 1.0);
  if (aspect >= 1.0f) {
    gl_Position = vec4((pointPos.x  * scale) / aspect, pointPos.y * scale, pointPos.z, 1.0);
  } else {
    gl_Position = vec4(pointPos.x  * scale, (pointPos.y * scale) / aspectY, pointPos.z, 1.0);
  }
  */
  gl_Position = vec4(pointPos.xy * scale, pointPos.z, 1.0);
  fragTexCoord = vec2(tileIndex) / vec2(tileCounts);
  //vec2 tilesetPos = vec2(tileIndex) / vec2(tileCounts);
  //vec3 relativeVertPos = (pointPos + 1.0f) / 2.0f;
  //vec2 vertIncrement = relativeVertPos / 32.0f;
  //vec3 vertIncrement = (vertTexCoord / 32.0f, 0.0f);
  //vec2 fragTexCoord = instancePos.xy; //tilesetPos + vertTexCoord
  // gl_Position = vec4(pointPos * instancePos, 1.0);
  //gl_Position = vec4(instancePos + vertIncrement, 1.0);

  // This might be a quick and dirty way to make the points stay the same
  // size on the screen regardless of Z distance.
  // gl_PointSize = PointSize / Position.w;
}
