#define GLFW_INCLUDE_NONE
#include <glbinding/gl/gl.h>
using namespace gl;

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include "Camera.h"
#include "transform.h"




Camera::Camera(int _width, int _height, glm::vec3 _position, glm::vec3 _rotation)
: width(_width), height(_height), position(_position), rotation(_rotation), zoom(25.0f), m_mode(false), m_press(false)
{}



void Camera::drawCam(float rx, float ry, float front, float back, int shaderId, char* uniform)
{
	int loc = 0;
	loc = glGetUniformLocation(shaderId, uniform);
	m_proj = Perspective(rx, ry, front, back);
	//m_view = Translate(position.x, position.y, zoom * -1.0f) * Rotate(0, rotation.y - 90.0f) * Rotate(2, rotation.x);
	if(!m_mode)
		m_view = LookAt(glm::vec3(25.0f, -25.0f, 25.0f), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
	else
		m_view = LookAt(glm::vec3(position.x, position.y, 0.0), glm::vec3(position.x, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
	glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(m_proj * m_view));
}

void Camera::Input(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		position.y += 0.1f;
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position.y -= 0.1f;
	}
	else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position.x += 0.1f;
	}
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position.x -= 0.1f;
	}
	
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
		m_press = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE && m_press)
	{
		m_press = false;
		m_mode = !m_mode;
	}
}