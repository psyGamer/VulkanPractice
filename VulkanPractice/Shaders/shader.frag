#version 450

layout(location = 0) in vec2 vUV;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec3 vViewVector;
layout(location = 4) in vec3 vLightVector;

layout(location = 0) out vec4 fColor;

layout(binding = 1) uniform sampler2D tex;

void main() {
	//fColor = texture(tex, vUV) * vec4(vColor, 1.0);
	//fColor = texture(tex, vUV * 3.0);

	vec3 N = normalize(vNormal);
	vec3 V = normalize(vViewVector);
	vec3 L = normalize(vLightVector);
	vec3 R = reflect(L, N);

	vec3 ambient = vColor * 0.1;
	vec3 diffuse = max(dot(N, L), 0.0) * vColor;
	vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(1.35);

	fColor = vec4(ambient + diffuse + specular, 1.0);
}