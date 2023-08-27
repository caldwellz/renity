#version 300 es
layout (location = 0) in vec3 aPos;

// These are built-in variables (see section 7.1), but they're explicitly declared here as a reminder
in highp int gl_VertexID;
in highp int gl_InstanceID;
out highp vec4 gl_Position;
out highp float gl_PointSize;

void main()
{
    gl_Position = vec4(aPos.xyz, 1.0);
    // gl_PointSize = float(gl_VertexID) * 5.0;
}
