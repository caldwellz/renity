#version 300 es
precision mediump float;

// These are built-in variables (see section 7.2), but they're explicitly declared here as a reminder
in highp vec4 gl_FragCoord;
in mediump vec2 gl_PointCoord;
out highp float gl_FragDepth;

// Color is the only required output, but can be declared with any name other than gl_FragColor for historical reasons
out vec4 fragColor;

// Fill in this data using GL_ShaderProgram::setUniformBlock("ColorBlock", {...})
layout (std140) uniform ColorBlock
{
  vec4 myColor;
};

void main()
{
    fragColor = myColor;

    // If depth buffering is enabled, this is the default if the shader does not otherwise touch gl_FragDepth anywhere
    gl_FragDepth = gl_FragCoord.z;
}
