#pragma once
#ifndef _TRANSFORMCOMPONENT_
#define _TRANSFORMCOMPONENT_
#include <vector>
#include <string>
#include "ComponentBase.h"
#include <glm/glm.hpp>

namespace MXRender
{

	class TransformComponent:public ComponentBase
	{
	private:
	protected:
		glm::vec3 rotation;
		glm::vec3 translation;
		glm::vec3 scale;

		glm::mat4 model_matrix;
		glm::mat4 relative_model_matrix;

		void update_relative_model_matrix();
	public:

		TransformComponent();
		virtual ~TransformComponent();
		void set_translation(glm::vec3 new_translation);
		void set_rotation(glm::vec3 new_rotation);
		void set_euler_rotation(glm::vec3 new_rotation);
		void set_scale(glm::vec3 new_scale);
		void set_translation_rotation_scale(glm::vec3 new_translation, glm::vec3 new_rotation, glm::vec3 new_scale);
		glm::mat4 get_rotation_matrix();
		glm::mat4 get_translation_matrix();
		glm::mat4 get_scale_matrix();
		//因为目前还没有父子关系，所以拿到的只是相对的matrix
		glm::mat4 get_model_matrix();
		glm::mat4 get_relative_model_matrix();

		glm::vec3 get_translation();
		glm::vec3 get_scale();
		glm::vec3 get_rotation();

		virtual void on_start() override;


		virtual void on_update() override;


		virtual void update(float delta_time) override;


		virtual void on_end() override;


		virtual void on_destroy() override;

	};

}
#endif 
