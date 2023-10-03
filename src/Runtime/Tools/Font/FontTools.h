#pragma once
#ifndef _FONTTOOLS_
#define _FONTTOOLS_
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "glm/fwd.hpp"
#include "msdfgen/msdfgen.h"
#include "ImportFont.h"

namespace MXRender { class VK_Texture; }
namespace msdfgen { class FontHandle; }


namespace MXRender
{
	class FontTools
	{
	public:
		static msdfgen::Bitmap<float, 1>* GenerateSdf(const msdfgen::FontHandle& fontFace, float fontSize, int charcode, glm::vec2 size, float radius);
		static msdfgen::Bitmap<float, 4>* GenerateMTSdf(const msdfgen::FontHandle & fontFace, float fontSize, int charcode, glm::vec2 size, float radius);
		static void TranslateBitmapToVkTexture(VK_Texture* out_texture, msdfgen::Bitmap<float, 1>& bitmap);
	protected:
	private:
	};
}
#endif 
