#version 450

layout(location = 0) in vec2 vUV;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec3 vViewVector;
layout(location = 4) in vec3 vLightVector;

layout(location = 0) out vec4 fColor;

layout(binding = 1) uniform sampler2D tex;
layout(binding = 2) uniform sampler2D texNormal;

layout(push_constant) uniform PushConstants {
	uint shadingMode;
} pushConstants;

void main() {
	/* Texture Shader
	fColor = texture(tex, vUV) * vec4(vColor, 1.0);
	fColor = texture(tex, vUV * 3.0);
	*/

	vec3 texColor = texture(tex, vUV).xyz;

	vec3 N = normalize(texture(texNormal, vUV).xyz);
	vec3 V = normalize(vViewVector);
	vec3 L = normalize(vLightVector);
	vec3 R = reflect(-L, N);

	// Phong Shader
	vec3 ambient = texColor * 0.1;
	vec3 diffuse = max(dot(N, L), 0.0) * texColor;
	vec3 specular = pow(max(dot(R, V), 0.0), 4.0) * vec3(1.035);

	vec3 phongColor = ambient + diffuse + specular;
	vec3 cartoonColor;

	// Simple Cartoon Shader
	if (pow(max(dot(R, V), 0.0), 5.0) > 0.5) {
		cartoonColor = texColor * 3.0;
	} else if (max(dot(V, N), 0.0) < 0.5) {
		cartoonColor = texColor / 10.0;
	} else if (max(dot(N, L), 0.0) > 0.1) {
		cartoonColor = texColor;
	} else {
		cartoonColor = texColor / 5.0;
	}

	if (pushConstants.shadingMode == 1) {
		fColor = vec4(phongColor, 1.0);
	} else if (pushConstants.shadingMode == 2) {
		fColor = vec4(cartoonColor, 1.0);
	} else {
		fColor = vec4(texColor, 1.0);
	}
}