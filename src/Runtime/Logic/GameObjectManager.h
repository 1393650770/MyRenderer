#pragma once
#ifndef _GAMEOBJECTMANAGER_
#define _GAMEOBJECTMANAGER_
#include <vector>
#include <string>


#include "GameObject.h"

namespace MXRender
{

	class GameObjectManager 
	{
	private:
	protected:

	public:
		std::vector<GameObject> object_list;
		GameObjectManager();
		virtual ~GameObjectManager();
		void destroy_object_list(GraphicsContext* context);
	};

}
#endif 
