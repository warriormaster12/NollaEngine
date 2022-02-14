//we will be using glsl version 4.5 syntax
#version 450

layout (set = 0, binding = 0) uniform Camera {
	mat4 render_matrix;
}camera;


void main()
{
	//const array of positions for the triangle
	const vec3 positions[3] = vec3[3](
		vec3(1.f,1.f, 0.0f),
		vec3(-1.f,1.f, 0.0f),
		vec3(0.f,-1.f, 0.0f)
	);
	//output the position of each vertex
	gl_Position = camera.render_matrix * vec4(positions[gl_VertexIndex], 1.0f);
}

