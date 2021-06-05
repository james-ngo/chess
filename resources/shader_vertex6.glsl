#version 330 core
layout(location = 0) in vec3 vertPos;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
out vec3 vertex_pos;
void main()
{
	vertex_pos = vertPos.xyz;

	vec4 tpos =  M * vec4(vertPos, 1.0);	
	gl_Position = P * V * tpos;
}
