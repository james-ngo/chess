#version 330 core
out vec4 color;
in vec3 vertex_pos;

in vec3 vertex_normal;
in vec2 vertex_tex;
uniform vec3 campos;

void main()
{
	//color = vec3(1,1,1);
	color.rgb = vec3(.1,.1,.1);
	color.a = 1;

	vec3 n = normalize(vertex_normal);
	vec3 lp = vec3(-10, -75, -10);

	vec3 ld = normalize(vertex_pos - lp);
	float diffuse = dot(n, ld);

	color.rgb *= diffuse * 1.2;
}
