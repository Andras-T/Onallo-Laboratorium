#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 0) out vec4 pos;
layout(location = 1) out vec2 coord;

void main() {
  gl_Position = inPosition;
  pos = inPosition;
  coord = (inPosition.xy + vec2(1.0f)) * 0.5f;
}