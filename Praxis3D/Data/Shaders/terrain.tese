#version 400 core

layout(quads, fractional_odd_spacing, ccw) in;

out vec2 texCoord;
out float depth;

uniform mat4 MVP;

uniform sampler2D diffuseTexture;

void main(void)
{
        float u = gl_TessCoord.x;
        float v = gl_TessCoord.y;

        vec4 a = mix(gl_in[1].gl_Position, gl_in[0].gl_Position, u);
        vec4 b = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, u);
        vec4 position = mix(a, b, v);
        texCoord = position.xy;
        float height = texture(diffuseTexture, texCoord).r;
        gl_Position = MVP * vec4(1.0, 1.0, 1.0, 1.0);
        depth = gl_Position.z;
}