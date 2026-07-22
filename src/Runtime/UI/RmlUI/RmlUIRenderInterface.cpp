#include "RmlUIRenderInterface.h"
#include "UI/UIRenderer.h"

#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/Vertex.h>
#include <RmlUi/Core/Types.h>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
MYRENDERER_BEGIN_NAMESPACE(RmlUI)

// =========================================================================
// Internal Impl that actually derives from Rml::RenderInterface
// =========================================================================
class RmlUIRenderInterface::Impl : public Rml::RenderInterface
{
public:
	UI::UIRenderer* renderer = nullptr;

	explicit Impl(UI::UIRenderer* r) : renderer(r) {}

	// === Required ===
	Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override
	{
		if (!renderer) return 0;
		UInt32 h = renderer->CompileGeometry(
			vertices.data(), static_cast<UInt32>(vertices.size()), sizeof(Rml::Vertex),
			indices.data(), static_cast<UInt32>(indices.size()), true);
		return static_cast<Rml::CompiledGeometryHandle>(h);
	}

	void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override
	{
		if (!renderer) return;
		// Pack translation into a 2-float array
		float tl[2] = { translation.x, translation.y };
		renderer->DrawGeometry(
			static_cast<UInt32>(geometry),
			nullptr, // transform is set separately via SetTransform
			static_cast<UInt32>(texture));
	}

	void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override
	{
		if (!renderer) return;
		renderer->ReleaseGeometry(static_cast<UInt32>(geometry));
	}

	Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override
	{
		if (!renderer) return 0;

		// Load file via RmlUI FileInterface
		auto* file_iface = Rml::GetFileInterface();
		if (!file_iface) return 0;

		auto fh = file_iface->Open(source);
		if (!fh) return 0;

		size_t total_size = file_iface->Length(fh);
		Rml::UniquePtr<Rml::byte[]> data = Rml::MakeUnique<Rml::byte[]>(total_size);
		file_iface->Read(data.get(), total_size, fh);
		file_iface->Close(fh);

		// Parse minimal TGA header (supports uncompressed RGB/RGBA 24/32 bpp)
		// Format: [id_len(1)][cmap_type(1)][img_type(1)][cmap_spec(5)][x_orig(2)][y_orig(2)][w(2)][h(2)][bpp(1)][img_desc(1)]
		if (total_size < 18) return 0;
		const Rml::byte* buf = data.get();
		uint8_t img_type = buf[2];
		if (img_type != 2) return 0; // Only support uncompressed true-color

		int w = buf[12] | (buf[13] << 8);
		int h = buf[14] | (buf[15] << 8);
		int bpp = buf[16];
		if (bpp != 24 && bpp != 32) return 0;

		int pixel_bytes = bpp / 8;
		int data_offset = 18 + buf[0]; // skip ID field
		const Rml::byte* pixel_buf = buf + data_offset;

		// Convert to RGBA8 premultiplied
		Rml::Vector<Rml::byte> rgba8;
		rgba8.resize(static_cast<size_t>(w) * h * 4);
		for (int i = 0; i < w * h; i++)
		{
			int src_idx = i * pixel_bytes;
			int dst_idx = i * 4;
			// TGA is BGR(A) order
			if (bpp == 24)
			{
				rgba8[dst_idx + 0] = pixel_buf[src_idx + 2]; // R ← B
				rgba8[dst_idx + 1] = pixel_buf[src_idx + 1]; // G ← G
				rgba8[dst_idx + 2] = pixel_buf[src_idx + 0]; // B ← R
				rgba8[dst_idx + 3] = 255;                     // A = opaque
			}
			else // bpp == 32
			{
				rgba8[dst_idx + 0] = pixel_buf[src_idx + 2];
				rgba8[dst_idx + 1] = pixel_buf[src_idx + 1];
				rgba8[dst_idx + 2] = pixel_buf[src_idx + 0];
				rgba8[dst_idx + 3] = pixel_buf[src_idx + 3];
			}
		}

		texture_dimensions.x = w;
		texture_dimensions.y = h;

		UInt32 hdl = renderer->CreateTexture(rgba8.data(), static_cast<UInt32>(w), static_cast<UInt32>(h));
		return static_cast<Rml::TextureHandle>(hdl);
	}

	Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override
	{
		if (!renderer) return 0;
		UInt32 hdl = renderer->CreateTexture(
			source.data(),
			static_cast<UInt32>(source_dimensions.x),
			static_cast<UInt32>(source_dimensions.y));
		return static_cast<Rml::TextureHandle>(hdl);
	}

	void ReleaseTexture(Rml::TextureHandle texture) override
	{
		if (!renderer) return;
		renderer->ReleaseTexture(static_cast<UInt32>(texture));
	}

	void EnableScissorRegion(bool enable) override
	{
		if (!renderer) return;
		renderer->EnableScissor(enable);
	}

	void SetScissorRegion(Rml::Rectanglei region) override
	{
		if (!renderer) return;
		renderer->SetScissor(region.Left(), region.Top(),
			static_cast<UInt32>(region.Width()),
			static_cast<UInt32>(region.Height()));
	}

	// === Optional: Clip mask ===
	void EnableClipMask(bool enable) override
	{
		if (!renderer) return;
		renderer->EnableClipMask(enable);
	}

	void RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation) override
	{
		if (!renderer) return;
		renderer->RenderToClipMask(
			static_cast<Int32>(operation),
			static_cast<UInt32>(geometry),
			nullptr);
	}

	// === Optional: Transform ===
	void SetTransform(const Rml::Matrix4f* transform) override
	{
		if (!renderer) return;
		if (transform)
		{
			// Rml::Matrix4f may be row-major or column-major depending on config.
			// Our renderer expects column-major float[16].
			// RmlUI matrices with RMLUI_MATRIX_ROW_MAJOR not defined: column-major.
			renderer->SetTransform(transform->data());
		}
		else
		{
			renderer->SetTransform(nullptr);
		}
	}

	// === Optional: Layers (stub for v1) ===
	// These will be connected to the renderer's layer stack in Phase 3.
};

// =========================================================================
// RmlUIRenderInterface
// =========================================================================
RmlUIRenderInterface::RmlUIRenderInterface(UI::UIRenderer* renderer)
{
	m_impl = new Impl(renderer);
}

void* RmlUIRenderInterface::GetRmlInterface() CONST
{
	return static_cast<Rml::RenderInterface*>(m_impl);
}

void RmlUIRenderInterface::SetRenderer(UI::UIRenderer* renderer)
{
	if (m_impl)
	{
		m_impl->renderer = renderer;
	}
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
