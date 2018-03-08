#version 450

//layout(location = 1) in vec4 vcol;
layout(location = 0) out vec4 out_color;
const vec4 vcol = vec4(0.1, 1.0, 1.0, 1.0);

/*layout(std140, binding = 0) uniform Ubo {
	vec4 color;
} u;*/

vec4 ambient(float num, vec4 c);

void main()
{
	out_color = ambient(0.1, vcol);// + u.color);
}
