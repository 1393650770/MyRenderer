#include "RenderUtils.h"
#include "RenderRource.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
namespace MXRender
{
    unsigned RenderUtils::Get_API_DataTypeEnum_To_OS_Size(ENUM_RENDER_DATA_TYPE data_type)
    {
        switch (data_type)
        {
        case MXRender::ENUM_RENDER_DATA_TYPE::Float:
        case MXRender::ENUM_RENDER_DATA_TYPE::Half:
        case MXRender::ENUM_RENDER_DATA_TYPE::Mat3:
        case MXRender::ENUM_RENDER_DATA_TYPE::Mat4:
            return sizeof(float);
            break;
        case MXRender::ENUM_RENDER_DATA_TYPE::Int:
        case MXRender::ENUM_RENDER_DATA_TYPE::Uint8:
        case MXRender::ENUM_RENDER_DATA_TYPE::Uint10:
        case MXRender::ENUM_RENDER_DATA_TYPE::Int16:
            return sizeof(int);
            break;
        case MXRender::ENUM_RENDER_DATA_TYPE::Bool:
            return sizeof(bool);
            break;
        default:
            return 0;
            break;
        }
        return 0;
    }

	std::shared_ptr<MXRender::TextureData> RenderUtils::Load_Texture(const std::string& texture_file_path,bool is_srgb)
	{


		std::shared_ptr<TextureData> texture = std::make_shared<TextureData>();

		int iw, ih, n;
		texture->pixels = stbi_load(texture_file_path.c_str(), &iw, &ih, &n, 4/*STBI_rgb_alpha*/);

		if (!texture->pixels)
			return nullptr;

		texture->width = iw;
		texture->height = ih;
		texture->format = (is_srgb) ? ENUM_TEXTURE_FORMAT::RGBA8S :
            ENUM_TEXTURE_FORMAT::RGBA8U;
		texture->depth = 1;
		texture->array_layers = 1;
		texture->mip_levels = 1;
		texture->type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;

		return texture;
	}
        
	

}