#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class Camera {

private:
	glm::vec3 m_Position = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec2 m_Rotation = glm::vec2(0.0f, 0.0f);

	float m_Fov = 70.0f;

	glm::vec3 m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_Up = glm::vec3(0.0f, -1.0f, 0.0f);

public:
	void Move(glm::vec3 positionDelta);
	void Rotate(glm::vec2 rotationDelta);
	void ChangeFOV(float fovDelta);

	void SetPosition(glm::vec3 position);
	void SetRotation(glm::vec2 rotation);
	void SetYaw(float yaw);
	void SetPitch(float pitch);
	void SetFOV(float fov);

	void SetFront(glm::vec3 front);
	void SetUp(glm::vec3 up);

	glm::vec3 GetPosition();
	glm::vec2 GetRotation();
	float GetYaw();
	float GetPitch();
	float GetFOV();

	glm::vec3 GetFront();
	glm::vec3 GetUp();
};