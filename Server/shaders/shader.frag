#version 450

layout(location = 0) in vec4 pos;
layout(location = 0) out vec4 outColor;

void main() {
	ivec2 p = ivec2(int((pos.x + 1.0f) * 100.0f), int((pos.y + 1.0f) * 100.0f));
	if (p.x % 8 < 4) {
		if (p.y % 8 < 4) {
			outColor = vec4(0.0f, 0.0f, 1.0f, 0.0f);
		} else {
			outColor = vec4(1.0f, 0.0f, 0.0f, 0.0f);
		}
	} else {
		if (p.y % 8 < 4) {
			outColor = vec4(1.0f, 0.0f, 0.0f, 0.0f);
		} else {
			outColor = vec4(0.0f, 0.0f, 1.0f, 0.0f);
		}
	}
}