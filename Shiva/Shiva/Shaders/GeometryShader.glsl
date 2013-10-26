#version 330 compatibility
layout(triangles) in;
layout(triangle_strip, max_vertices=6) out;

smooth in vec4 vColor[];
smooth in vec4 vPos[];

smooth out vec4 vColor2;
smooth out vec4 vPos2;

void main()
{	
  for(int i=0; i<3; i++)
  {
    gl_Position = gl_in[i].gl_Position;
	vColor2 = vColor[i];
	vPos2 = vPos[i];
    EmitVertex();
  }
  EndPrimitive();

  for(int i=0; i<3; i++)
  {
    gl_Position = gl_in[i].gl_Position + vec4(2,0,0,0);
	vColor2 = 1 - vColor[i];
	vPos2 = vPos[i] + vec4(2,0,0,0);
    EmitVertex();
  }
  EndPrimitive();
}  