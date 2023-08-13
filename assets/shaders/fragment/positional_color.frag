#version 300 es
precision mediump float;

// These are built-in variables (see section 7.2), but they're explicitly declared here as a reminder
in highp vec4 gl_FragCoord;
in mediump vec2 gl_PointCoord;
// in bool gl_FrontFacing;
out highp float gl_FragDepth;

// Color is the only required output, but can be declared with any name other than gl_FragColor for historical reasons
out vec4 fragColor;

void main()
{
    // Can discard areas using frag shader, but not vertex in ES
    // if (gl_FragCoord.x < 900.0f) { discard; }

    fragColor = vec4(gl_FragCoord.x / 1920.0f, gl_FragCoord.y / 1080.0f, gl_FragCoord.z, 0.7f);

    // If depth buffering is enabled, this is the default if the shader does not otherwise touch gl_FragDepth anywhere
    gl_FragDepth = gl_FragCoord.z;
}
