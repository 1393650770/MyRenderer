#include "FontTools.h"
#include <memory>
#include "../../RHI/Vulkan/VK_Texture.h"
#include "../../RHI/RenderRource.h"
#include <algorithm>


msdfgen::Bitmap<float, 1>* MXRender::FontTools::GenerateSdf(const msdfgen::FontHandle& fontFace, float fontSize, int charcode, glm::vec2 size, float radius)
{
	const auto ftFace = (fontFace.face);
	msdfgen::FontHandle* fontPtr = &(const_cast<msdfgen::FontHandle&>(fontFace));
	msdfgen::Shape shape;
	if (msdfgen::FontHandle::loadGlyph(shape, fontPtr, (msdfgen::unicode_t)charcode) && !shape.contours.empty()) {
		//const double scale = fontSize / (2048 / 64 * 2048 / ftFace->units_per_EM);
		//const double scale = fontSize * ftFace->units_per_EM / 65536; // ???
		const double scale = fontSize / (ftFace->units_per_EM / 64);
		const double range = 2.0f * radius / scale;
		const auto pos = glm::vec2(radius / scale, radius / scale);

		shape.inverseYAxis = true;
		shape.normalize();
		const auto bounds = shape.getBounds();

		edgeColoringSimple(shape, 3.0);
		msdfgen::Projection projection({ scale, scale }, { pos.x - bounds.l, pos.y - bounds.b });
		msdfgen::GeneratorConfig config;
		auto bmp = new msdfgen::Bitmap<float, 1>(size.x, size.y);
		msdfgen::generateSDF(*bmp, shape, projection, range, config);
		return bmp;
	}
	return nullptr;
}

msdfgen::Bitmap<float, 4>* MXRender::FontTools::GenerateMTSdf(const msdfgen::FontHandle& fontFace, float fontSize, int charcode, glm::vec2 size, float radius)
{
	const auto ftFace = (fontFace.face);
	msdfgen::FontHandle* fontPtr = &(const_cast<msdfgen::FontHandle&>(fontFace));
	msdfgen::Shape shape;
	if (msdfgen::FontHandle::loadGlyph(shape, fontPtr, (msdfgen::unicode_t)charcode) && !shape.contours.empty()) {
		//const double scale = fontSize / (2048 / 64 * 2048 / ftFace->units_per_EM);
		//const double scale = fontSize * ftFace->units_per_EM / 65536; // ???
		const double scale = fontSize / (ftFace->units_per_EM / 64);
		const double range = 2.0f * radius / scale;
		const auto pos = glm::vec2(radius / scale, radius / scale);

		shape.inverseYAxis = true;
		shape.normalize();
		const auto bounds = shape.getBounds();

		edgeColoringSimple(shape, 3.0);
		msdfgen::Projection projection({ scale, scale }, { pos.x - bounds.l, pos.y - bounds.b });

		msdfgen::MSDFGeneratorConfig config;
		auto bmp =new msdfgen::Bitmap<float, 4>(size.x, size.y);
		msdfgen::generateMTSDF(*bmp, shape, projection, range, config);
		return bmp;

	}
	return nullptr;
}

void MXRender::FontTools::TranslateBitmapToVkTexture(VK_Texture* out_texture,msdfgen::Bitmap<float, 1>& bitmap)
{
	std::vector<unsigned char> pixels(bitmap.width() * bitmap.height());
	std::vector<unsigned char>::iterator it = pixels.begin();
	for (int y = bitmap.height() - 1; y >= 0; --y)
		for (int x = 0; x < bitmap.width(); ++x)
			*it++ = (unsigned char)std::clamp(256.f * (*bitmap(x, y)), 0.f, 255.f);
	TextureData data;
	data.pixels= pixels.data();
	data.width=bitmap.width();
	data.height=bitmap.height();
	data.array_layers=1;
	data.mip_levels=1;
	data.depth=1;
	data.is_free_by_me=false;
	data.format=ENUM_TEXTURE_FORMAT::R8;


	out_texture->updata(&data);

}

