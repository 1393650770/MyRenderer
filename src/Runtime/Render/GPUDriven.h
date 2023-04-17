#pragma once
#ifndef _GPUDRIVEN_
#define _GPUDRIVEN_
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
namespace MXRender
{
	struct GPUObjectData {
		glm::mat4 modelMatrix;
		glm::vec4 origin_rad; // bounds
		glm::vec4 extents;  // bounds
	};

	class GPUDrivenSystem
	{
	public:
		GPUDrivenSystem();
		virtual ~GPUDrivenSystem();
		void excute_upload_computepass();
	protected:
	private:
	};
}
#endif //_GPUDRIVEN_