#version 330 compatibility
layout(triangles) in;
layout(triangle_strip, max_vertices=9) out;

uniform float threshold;

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
	//vColor2 = vec4(i == 0 ? 1 : 0, i == 1 ? 1 : 0, i == 2 ? 1 : 0, 1.0);
	vPos2 = vPos[i];
    EmitVertex();
  }
  EndPrimitive();

  // above = 0, below = 1
  int v0_ab = vColor[0].r < threshold ? 1 : 0;
  int v1_ab = vColor[1].r < threshold ? 1 : 0;
  int v2_ab = vColor[2].r < threshold ? 1 : 0;

  int sum = v0_ab + v1_ab + v2_ab;
  
  int numTris = 2 - abs(sum - 2);

  vec4 col1 = vec4(0,1,0,0.5);
  vec4 col2 = vec4(1,0,0,0.5);
  vec4 col3 = vec4(0,0,1,0.5);

  // populate the lerp positions
  float lerps[3]; // one extra for overflow
  int index = 0;
  for(int i = 0; i < 3; i++)
  {
	float currentCol = vColor[i].r;
	float nextCol = vColor[(i+1) % 3].r;

	float l = (threshold - nextCol) / (currentCol - nextCol);
	lerps[index] = currentCol < nextCol ? l : 1.0 - l;
	index += (l > 1.0f || l < 0.0f) ? 0 : 1;
  }

  vec4 zshift = vec4(0,0,-.1,0);

  if(sum == 1)
  {
	int oneTriShift = v0_ab + v1_ab + v1_ab + v2_ab + v2_ab + v2_ab - 1; // 0 for v0 used, 1 for v1 used, 2 for v2 used
	int prev = (oneTriShift + 2) % 3;
	int next = (oneTriShift + 1) % 3;

	// I have no idea why that flip is necessary, from a theoretical standpoint
	vec4 lerp0 = mix(gl_in[prev].gl_Position, gl_in[oneTriShift].gl_Position, oneTriShift == 0 ? lerps[1] :lerps[0]);
	vec4 lerp1 = mix(gl_in[next].gl_Position, gl_in[oneTriShift].gl_Position, oneTriShift == 0 ? lerps[0] :lerps[1]);

	gl_Position = lerp0 + zshift;
	vColor2 = col1;
	vPos2 = gl_Position + zshift;
    EmitVertex();

	gl_Position = gl_in[oneTriShift].gl_Position + zshift;
	vColor2 = col1;
	vPos2 = gl_Position + zshift;
    EmitVertex();

	gl_Position = lerp1 + zshift;
	vColor2 = col1;
	vPos2 = gl_Position + zshift;
    EmitVertex();
	EndPrimitive();
  }

  if(sum == 2)
  {
	int twoTriMissing = 3 - (v1_ab + v2_ab + v2_ab);
	int prev = (twoTriMissing + 2) % 3;
	int next = (twoTriMissing + 1) % 3;

	vec4 lerp0 = mix(gl_in[twoTriMissing].gl_Position, gl_in[prev].gl_Position, twoTriMissing == 0 ? lerps[1] :lerps[0]);
	vec4 lerp1 = mix(gl_in[twoTriMissing].gl_Position, gl_in[next].gl_Position, twoTriMissing == 0 ? lerps[0] :lerps[1]);

	// triangle 1
	gl_Position = lerp0 + zshift;
	vColor2 = col2;
	vPos2 = gl_Position + zshift;
	EmitVertex();

	gl_Position = lerp1 + zshift;
	vColor2 = col2;
	vPos2 = gl_Position + zshift;
	EmitVertex();

	gl_Position = gl_in[prev].gl_Position + zshift;
	vColor2 = col2;
	vPos2 = gl_Position + zshift;
	EmitVertex();
	EndPrimitive();

	// triangle 2
	gl_Position = lerp1 + zshift;
	vColor2 = col2;
	vPos2 = gl_Position + zshift;
	EmitVertex();

	gl_Position = gl_in[next].gl_Position + zshift;
	vColor2 = col2;
	vPos2 = gl_Position + zshift;
	EmitVertex();

	gl_Position = gl_in[prev].gl_Position + zshift;
	vColor2 = col2;
	vPos2 = gl_Position + zshift;
	EmitVertex();
	EndPrimitive();
  }

  if(sum == 3)
  {
	for(int i=0; i<3; i++)
	{
		gl_Position = gl_in[i].gl_Position + zshift;
		vColor2 = col3;
		vPos2 = vPos[i] + zshift;
		EmitVertex();
	}
	EndPrimitive();
  }
  

 
}  
