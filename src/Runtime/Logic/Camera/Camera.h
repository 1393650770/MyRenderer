#pragma once
#ifndef _CAMERA_
#define _CAMERA_
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "glm/fwd.hpp"
#include "../Object.h"
#include "../Component/TransformComponent.h"


namespace MXRender
{
	enum class ENUM_CAMERA_TYPE
	{
		NONE=0,
		PROJECTION,
		ORTHO,

		COUNT

	};
	class Camera :public Object
	{
	private:
		void calc_proj_mat();
		void calc_otho_mat();
	protected:
		TransformComponent* transform;
		float fov, far , near , movement_speed,width, height;
		glm::vec3 direction,up,right, focal_point;
		ENUM_CAMERA_TYPE camera_type;
		glm::mat4 projection_mat,view_mat;
		void calc_projection_mat();
	public:
		void set_position(glm::vec3 new_position);
		void set_direction(glm::vec3 new_direction);
		void set_fov(float new_fov);
		void set_far(float new_far);
		void set_near(float new_near);
		void set_speed(float new_speed);
		void set_width(float new_width);
		void set_height(float new_height);
		glm::vec3 get_position() const;
		glm::vec3 get_direction() const;
		glm::mat4 get_projection_mat() const ;
		glm::mat4 get_view_mat() ;
		void update_rotation(float x_offset,float y_offset);
		Camera();
		virtual ~Camera();

	};

}
#endif 
