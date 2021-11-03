#version 450

out gl_PerVertex {
	vec4 gl_Position;
};

layout(binding = 0) uniform UBO {
	mat4 Model;
	mat4 View;
	mat4 Proj;

	vec3 LightPosition;
} u;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec3 aNormal;

layout(location = 0) out vec2 vUV;
layout(location = 1) out vec3 vColor;
layout(location = 2) out vec3 vNormal;
layout(location = 3) out vec3 vViewVector;
layout(location = 4) out vec3 vLightVector;

void main() {
	vUV = aUV;

	vColor.r = u.Model[0][0] * aColor.r + 0.1;
	vColor.g = u.View[1][1] * aColor.g + 0.1;
	vColor.b = u.Proj[2][2] * aColor.b + 0.1;

	vec4 worldPos = u.Model * vec4(aPos, 1.0);
	
	vNormal = mat3(u.Model) * aNormal; // Get rid of translation of matrix
	vViewVector = (u.View * worldPos).xyz;
	vLightVector = u.LightPosition - worldPos.xyz;

	gl_Position = u.Proj * u.View * u.Model * vec4(aPos, 1.0);
}