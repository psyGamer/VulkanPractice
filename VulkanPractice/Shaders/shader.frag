#version 450

layout(location = 0) in vec2 vUV;
layout(location = 1) in vec3 vColor;

layout(location = 0) out vec4 fColor;

layout(binding = 1) uniform sampler2D tex;

void main() {
	//fColor = texture(tex, vUV) * vec4(vColor, 1.0);
	fColor = texture(tex, vUV);
}