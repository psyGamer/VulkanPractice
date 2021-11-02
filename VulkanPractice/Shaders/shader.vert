#version 450

out gl_PerVertex {
	vec4 gl_Position;
};

layout(binding = 0) uniform UBO {
	mat4 uModelViewProj;
} ubo;

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;

layout(location = 0) out vec3 vColor;

void main() {
	vColor.r = ubo.uModelViewProj[0][0] * aColor.r;
	vColor.g = ubo.uModelViewProj[1][1] * aColor.g;
	vColor.b = ubo.uModelViewProj[3][2] * aColor.b;

	gl_Position = ubo.uModelViewProj * vec4(aPos, 0.0, 1.0);
}