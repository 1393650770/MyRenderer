#pragma once

#ifndef _My_TEXTURE_
#define _My_TEXTURE_


#include"GLFW/glfw3.h"
#include <vector>
#include <string>

namespace MXRender
{
	struct MipmapInfo {
		size_t dataSize;
		size_t dataOffset;
	};
    
} // namespace name

#endif