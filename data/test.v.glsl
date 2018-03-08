#version 450

layout(location = 0) in vec4 vertex;
layout(location = 1) out vec4 vcol;

void main()
{
	vcol = vec4(1.0, 0.0, 0.0, 1.0);
	gl_Position = vertex;
}
