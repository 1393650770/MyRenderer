#pragma once
#ifndef _UNIFORMBUFFER_
#define _UNIFORMBUFFER_
#include "glm/ext/matrix_float4x4.hpp"

struct MVP_Struct
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct CameraData
{
	alignas(16) glm::vec3 viewPos;
	alignas(16) glm::vec3 viewDir;
};

#endif