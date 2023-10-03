#pragma once
#ifndef _VK_TEXT_
#define _VK_TEXT_
#include <vector>
#include <string>
#include "TextBase.h"
#include <map>
#include "MeshBase.h"

namespace MXRender { class VK_GraphicsContext; }

namespace MXRender { class GraphicsContext; }


namespace MXRender
{
	struct Character {
        VK_Texture* text_texture;
		glm::ivec2 size;    // Size of glyph
		glm::ivec2 bearing;  // Offset from baseline to left/top of glyph
	};
    
	struct VK_TextInfo :public TextInfo
	{
    public:
        std::map<char, Character> Characters;
        std::map<char, VK_MeshInfo> MeshInfos;
	};
    class VK_Text :public TextBase
    {
    private:
        bool is_bedestroyed=false;
    protected:
        VK_TextInfo vk_text_info;

		void setup_vk_vertexbuffer(VK_GraphicsContext* cur_context);
		void setup_vk_indexbuffer(VK_GraphicsContext* cur_context);
    public:
        VK_Text();
        virtual ~VK_Text();
        virtual void load_ttf(const std::string& ttf_path) override final;
        virtual void destroy_text_info(GraphicsContext* context) override final;
        virtual void init_text_info(GraphicsContext* context) override final;

        VK_TextInfo& get_text_info();
        virtual void bind(GraphicsContext* context) override final;
        virtual void render(GraphicsContext* context) override final;
    };

}
#endif 
