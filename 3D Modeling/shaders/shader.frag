#version 450


layout(location=0) in vec3 fragColor;
layout(location=1) in vec2 fragTexCoord;

layout(binding=1) uniform sampler2D texSampler;

layout(location=0) out vec4 outColor;

void main()
{
	if(fragTexCoord.x == 1000000)
	{
		outColor = vec4(1.0,0.4,0.0,1.0);
	}
	else
	{
		outColor = vec4(fragColor*texture(texSampler,fragTexCoord).rgb,1.0);
	}
}
