#include "GameObjectManager.h"
#include "../RHI/RenderState.h"
#include "../RHI/RenderEnum.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../Mesh/MeshBase.h"



MXRender::GameObjectManager::GameObjectManager()
{
	object_list.emplace_back("Resource/Mesh/viking_room.obj");

	object_list.emplace_back("Resource/Mesh/rock.obj");

	object_list.emplace_back("Resource/Mesh/sponza.obj");

}

MXRender::GameObjectManager::~GameObjectManager()
{

}

void MXRender::GameObjectManager::destroy_object_list(GraphicsContext* context)
{
	switch (RenderState::render_api_type)
	{
	case ENUM_RENDER_API_TYPE::Vulkan:
	{
		VK_GraphicsContext* vk_context = dynamic_cast<VK_GraphicsContext*>(context);
		if (!vk_context ) return;
		
		for (int i=0;i<object_list.size();i++)
		{
			object_list[i].get_staticmesh()->get_mesh_data().lock()->destroy_mesh_info(context);
		}
		break;
	}

	default:
		break;
	}
}
