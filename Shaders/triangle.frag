//glsl version 4.5
#version 450

layout (location = 0) in vec3 inColor;

//output write
layout (location = 0) out vec4 outFragColor;

//push constants block
layout( push_constant ) uniform constants
{
	vec4 color;
} TriangleData;

layout (set = 0, binding = 0) uniform Camera {
	mat4 render_matrix;
}camera;


layout(set = 1, binding = 0) uniform another_test {
    vec4 color;
}test_data;

void main()
{
	//return red
	outFragColor = vec4(inColor, 1.0f);
}

