#include "InputSystem.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include "GLFW/glfw3.h"
#include "../GameObject.h"
#include "../Component/InputComponent.h"
#include "../Object.h"




MXRender::InputSystem::InputSystem()
{

}

MXRender::InputSystem::~InputSystem()
{

}


void MXRender::InputSystem::process_input(GLFWwindow* window)
{

	InputComponent* input_component = cur_controller_object->get_component<InputComponent>("InputComponent");
	if (input_component==nullptr)
	{
		return;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		input_component->run(GLFW_KEY_W, PRESS);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		input_component->run(GLFW_KEY_S, PRESS);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		input_component->run(GLFW_KEY_A, PRESS);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		input_component->run(GLFW_KEY_D, PRESS);
	if (glfwGetKey(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		input_component->run(GLFW_MOUSE_BUTTON_LEFT, PRESS);
	if (glfwGetMouseButton(window, 0) == GLFW_PRESS)
		input_component->run(GLFW_MOUSE_BUTTON_LEFT, PRESS);
}