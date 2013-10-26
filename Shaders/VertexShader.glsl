#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

uniform vec2 offset;
uniform float zNear;
uniform float zFar;
uniform float frustumScale;

smooth out vec4 vColor;
smooth out vec4 vPos;

void main()
{
	vec4 cameraSpacePos = position;
	// scale down the cube
	cameraSpacePos /= vec4(100,100,100,1);
	
	// apply offset and move forward
	cameraSpacePos += vec4(offset * 2, -3.0f, 0.0f);

	vPos = cameraSpacePos;

	vec4 clipSpacePos;

	// scale xy to simulate zoom
	clipSpacePos.xy = cameraSpacePos.xy * frustumScale;

	// depth adjustment of z coordinate
	clipSpacePos.z = cameraSpacePos.z * (zNear + zFar) / (zNear - zFar);
	clipSpacePos.z += 2 * zNear * zFar / (zNear-zFar);

	clipSpacePos.w = -cameraSpacePos.z;
	
	//gl_Position = cameraSpacePos;
	gl_Position = clipSpacePos;
	vColor = color;
}