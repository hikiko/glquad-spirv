#version 450

vec4 ambient(float num, vec4 c)
{
	float r = min(c.r + num, 1.0);
	float g = min(c.g + num, 1.0);
	float b = min(c.b + num, 1.0);
	float a = 1.0;

	return vec4(r, g, b, a);
}
