#include "TextBase.h"

MXRender::TextBase::TextBase()
{

}

MXRender::TextBase::~TextBase()
{

}

void MXRender::TextBase::update_content(const std::string& new_content)
{
	content = new_content;
	is_need_to_update=true;
}

void MXRender::TextBase::load_ttf(const std::string& ttf_path)
{

}

void MXRender::TextBase::destroy_text_info(GraphicsContext* context)
{

}

void MXRender::TextBase::init_text_info(GraphicsContext* context)
{

}

std::string& MXRender::TextBase::get_content()
{
	return content;
}
