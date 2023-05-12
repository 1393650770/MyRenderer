#pragma once
#ifndef _DEFAULTSETTING_
#define _DEFAULTSETTING_
#include <memory>

namespace MXRender { struct TaskSystem; }


namespace MXRender { class MaterialSystem; }

namespace MXRender { class TaskGraph; }

namespace MXRender { class TextureManager; }

namespace MXRender { class ThreadPool; }

namespace MXRender { class InputSystem; }


namespace MXRender { class GameObjectManager; }

namespace MXRender { class VK_GraphicsContext; }
namespace MXRender
{
	class DefaultSetting
	{
	public:
		std::shared_ptr <VK_GraphicsContext> context;
		std::shared_ptr <GameObjectManager> gameobject_manager;
		std::shared_ptr <InputSystem> input_system;
		std::shared_ptr <TaskSystem> task_system;
		std::shared_ptr <TextureManager> texture_manager;
		std::shared_ptr < MaterialSystem> material_system;
		DefaultSetting();
		virtual ~DefaultSetting();
		void destroy();
		void load_setting();
		int width = 1280;
		int height = 920;
		bool is_enable_dispatch= false;
		bool is_enable_gpu_driven = true;
		bool is_enable_debug_loop=false;
		bool is_enable_culling = true;
		bool is_enable_batch= false;
		int mianshu=0;
	private:

	};

}
#endif //_DEFAULTSETTING_