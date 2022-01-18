#ifndef CAMERA_H
#define CAMERA_H

class Camera
{
	public:
		Camera() = default;
		Camera(int _width, int _height, glm::vec3 _position, glm::vec3 _rotation);
		void drawCam(float rx, float ry, float front, float back, int shaderId, char* uniform);
		void Input(GLFWwindow* window);
		int width;
		int height;
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 target;
		float zoom;
		float spin;
		float tilt;

	private:
		glm::mat4 m_view;
		glm::mat4 m_proj;
		bool m_mode;
		bool m_press;

	private:

};

#endif

