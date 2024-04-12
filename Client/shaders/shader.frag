#version 450

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 coord;

layout(binding = 0) uniform sampler2D image;

layout(location = 0) out vec4 outColor;
	
void main() {
	float x = texture(image, coord).x;
	float y = texture(image, coord).y;
	float z = texture(image, coord).z;
	outColor = vec4(x, y, z, 0.0f);
}