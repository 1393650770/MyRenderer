#pragma once
#ifndef _TEXTBASE_
#define _TEXTBASE_
#include <vector>
#include <string>
#include "../RHI/Vulkan/VK_Texture.h"
#include "MeshBase.h"


namespace MXRender { class GraphicsContext; }


namespace MXRender
{
    struct SimpleVertex;
    struct TextInfo
    {

    };


    class TextBase:public MeshBase
    {
    private:
    protected:
        bool is_need_to_update=false;
        bool is_already_init_ttf=false;
        std::string content;
    public:
        TextBase();
        virtual ~TextBase();
        void update_content(const std::string& new_content);
        virtual void load_ttf(const std::string& ttf_path);
        virtual void destroy_text_info(GraphicsContext* context);
        virtual void init_text_info(GraphicsContext* context);
        std::string& get_content();
    };

}
#endif 
