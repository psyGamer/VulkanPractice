#include "Camera.h"

void Camera::Move(glm::vec3 positionDelta) {
	m_Position += positionDelta;
}
void Camera::Rotate(glm::vec2 rotationDelta) {
	m_Rotation.x -= rotationDelta.x;
	m_Rotation.y += rotationDelta.y;
}
void Camera::ChangeFOV(float fovDelta) {
	m_Fov += fovDelta;
}

void Camera::SetPosition(glm::vec3 position) {
	m_Position = position;
}

void Camera::SetRotation(glm::vec2 rotation) {
	m_Rotation = rotation;
}
void Camera::SetYaw(float yaw) {
	m_Rotation.x = -yaw;
}
void Camera::SetPitch(float pitch) {
	m_Rotation.y = pitch;
}
void Camera::SetFOV(float fov) {
	m_Fov = fov;
}

void Camera::SetFront(glm::vec3 front) {
	m_Front = front;
}
void Camera::SetUp(glm::vec3 up) {
	m_Up = up;
}

glm::vec3 Camera::GetPosition() {
	return m_Position;
}
glm::vec2 Camera::GetRotation() {
	return m_Rotation;
}
float Camera::GetYaw() {
	return m_Rotation.x;
}
float Camera::GetPitch() {
	return m_Rotation.y;
}
float Camera::GetFOV() {
	return m_Fov;
}

glm::vec3 Camera::GetFront() {
	return m_Front;
}
glm::vec3 Camera::GetUp() {
	return m_Up;
}
