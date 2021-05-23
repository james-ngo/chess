#version 330 core
out vec4 color;
in vec3 vertex_pos;
void main()
{
	//color = vec3(1,1,1);
	color.rgb = vec3(0, .2, 0);
	color.a = 1.0f;
}
