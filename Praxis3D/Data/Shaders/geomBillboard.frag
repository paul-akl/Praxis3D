#version 330 core

struct DirectionalLight
{
    vec3 direction;
};

in	vec2 texCoord;
out vec4 fragColor;

uniform	DirectionalLight directionalLight;
uniform sampler2D textureSampler;

void main()
{
    fragColor = texture2D(textureSampler, texCoord);

	//if(fragColor.r == 0 && fragColor.g == 0 && fragColor.r == 0)

	if(fragColor.a == 0.0)
	{
		discard;
	}
}