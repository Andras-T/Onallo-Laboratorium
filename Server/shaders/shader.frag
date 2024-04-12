#version 450

layout(location = 0) in vec4 pos;
layout(location = 0) out vec4 outColor;

void main() {
  outColor = vec4(pos.x, pos.y, 1.0f, 0.0f);
}