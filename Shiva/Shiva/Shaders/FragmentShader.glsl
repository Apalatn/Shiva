#version 330

smooth in vec4 vColor2;
smooth in vec4 vPos2;

out vec4 outputColor;

void main()
{
	outputColor = vColor2;
}
