//glsl version 4.5
#version 450

//output write
layout (location = 0) out vec4 outFragColor;

//push constants block
layout( push_constant ) uniform constants
{
	vec4 color;
} TriangleData;

void main()
{
	//return red
	outFragColor = vec4(TriangleData.color);
}

