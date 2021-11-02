#version 450

out gl_PerVertex {
	vec4 gl_Position;
};

layout(binding = 0) uniform UBO {
	mat4 uModelViewProj;
} ubo;

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec3 aColor;

layout(location = 0) out vec2 vUV;
layout(location = 1) out vec3 vColor;

void main() {
	vUV = aUV;

	vColor.r = ubo.uModelViewProj[0][0] * aColor.r + 0.1;
	vColor.g = ubo.uModelViewProj[1][1] * aColor.g + 0.1;
	vColor.b = ubo.uModelViewProj[3][2] * aColor.b + 0.1;

	gl_Position = ubo.uModelViewProj * vec4(aPos, 0.0, 1.0);
}