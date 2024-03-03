#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 0) out vec4 pos;

void main() {
  gl_Position = inPosition;
  pos = inPosition;
}