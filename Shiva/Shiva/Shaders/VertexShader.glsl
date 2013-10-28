#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

uniform mat4 projection;

smooth out vec4 vColor;
smooth out vec4 vPos;

void main()
{
	vPos = position;

	vec4 clipSpacePos =	projection * position;
	
	//gl_Position = cameraSpacePos;
	gl_Position = clipSpacePos;
	vColor = color;
}