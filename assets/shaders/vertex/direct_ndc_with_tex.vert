#version 300 es
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 vertTexCoord;
layout (location = 2) in vec3 instancePos;

out vec2 fragTexCoord;

void main()
{
    gl_Position = vec4(aPos + instancePos, 1.0);
    fragTexCoord = vertTexCoord;
}
