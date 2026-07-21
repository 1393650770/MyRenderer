#include "AndroidWindow.h"

#if PLATFORM_ANDROID

#include "RHI/RenderRHI.h"
#include "Render/RenderInterface.h"
#include "Render/Core/RenderFrameData.h"
#include "RHI/RenderViewport.h"
#include "RHI/RenderCommandList.h"
#include "Tool/ShaderLibrary.h"
#include <android/log.h>
#include <android/input.h>

#define LOG_TAG "MXRender"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

MYRENDERER_BEGIN_NAMESPACE(MXRender)

AndroidWindow* AndroidWindow::s_instance = nullptr;

AndroidWindow::AndroidWindow(struct android_app* in_app)
	: m_app(in_app)
{
	s_instance = this;
	m_app->userData = this;
	m_app->onAppCmd = &AndroidWindow::OnAppCmd;
	m_app->onInputEvent = &AndroidWindow::OnInputEvent;
}

AndroidWindow::~AndroidWindow()
{
	if (s_instance == this)
		s_instance = nullptr;
}

void AndroidWindow::OnAppCmd(struct android_app* in_app, int32_t cmd)
{
	AndroidWindow* self = STATIC_CAST(in_app->userData, AndroidWindow);
	if (self) self->HandleCmd(cmd);
}

void AndroidWindow::HandleCmd(int32_t cmd)
{
	switch (cmd)
	{
	case APP_CMD_INIT_WINDOW:
		m_native_window = m_app->window;
		LOGI("APP_CMD_INIT_WINDOW: window=%p", m_native_window);
		break;
	case APP_CMD_TERM_WINDOW:
		LOGI("APP_CMD_TERM_WINDOW");
		if (m_render) m_render->OnShutdown_Logic();
		ShutdownRHI();
		m_native_window = nullptr;
		break;
	case APP_CMD_GAINED_FOCUS:
		m_has_focus = true;
		break;
	case APP_CMD_LOST_FOCUS:
		m_has_focus = false;
		break;
	case APP_CMD_DESTROY:
		LOGI("APP_CMD_DESTROY");
		m_destroy_requested = true;
		break;
	case APP_CMD_WINDOW_RESIZED:
		LOGI("APP_CMD_WINDOW_RESIZED");
		break;
	default: break;
	}
}

int32_t AndroidWindow::OnInputEvent(struct android_app* in_app, AInputEvent* event)
{
	AndroidWindow* self = STATIC_CAST(in_app->userData, AndroidWindow);
	if (self) self->HandleMotionEvent(event);
	return 0;
}

void AndroidWindow::HandleMotionEvent(AInputEvent* event)
{
	if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_MOTION) return;

	Int action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
	Int pc = AMotionEvent_getPointerCount(event);
	if (pc > TouchState::kMaxPointers) pc = TouchState::kMaxPointers;

	m_touch_state.pointer_count = pc;
	for (Int i = 0; i < pc; ++i)
	{
		m_touch_state.pointers[i].x = AMotionEvent_getX(event, i);
		m_touch_state.pointers[i].y = AMotionEvent_getY(event, i);
		m_touch_state.pointers[i].id = AMotionEvent_getPointerId(event, i);
		m_touch_state.pointers[i].active = true;
	}
	for (Int i = pc; i < TouchState::kMaxPointers; ++i)
		m_touch_state.pointers[i].active = false;

	if (pc >= 2)
	{
		Float32 dx = m_touch_state.pointers[0].x - m_touch_state.pointers[1].x;
		Float32 dy = m_touch_state.pointers[0].y - m_touch_state.pointers[1].y;
		m_touch_state.pinch_distance = sqrtf(dx * dx + dy * dy);
	}
	else m_touch_state.pinch_distance = 0.0f;

	if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_CANCEL)
	{
		m_touch_state.pointer_count = 0;
		for (Int i = 0; i < TouchState::kMaxPointers; ++i)
			m_touch_state.pointers[i].active = false;
		m_touch_state.pinch_distance = 0.0f;
	}
}

void AndroidWindow::InitRHI()
{
	if (!m_native_window) { LOGE("InitRHI: null native window"); return; }

	m_width  = ANativeWindow_getWidth(STATIC_CAST(m_native_window, ANativeWindow));
	m_height = ANativeWindow_getHeight(STATIC_CAST(m_native_window, ANativeWindow));
	LOGI("InitRHI: %dx%d", m_width, m_height);

	RHIInit();

	if (m_app->activity && m_app->activity->assetManager)
		MXRender::Tool::ShaderLibrary::SetAssetManager(m_app->activity->assetManager);

	m_viewport = RHICreateViewport(m_native_window, m_width, m_height, false);

	if (m_render) m_render->OnInit_Logic(this, m_viewport);
}

void AndroidWindow::ShutdownRHI()
{
	LOGI("ShutdownRHI");
	if (m_viewport) { delete m_viewport; m_viewport = nullptr; }
	RHIShutdown();
}

void AndroidWindow::StartEventLoop()
{
	if (!m_render) return;

	Bool rhi_ok = false;
	while (!m_destroy_requested)
	{
		int ident, events;
		struct android_poll_source* source;
		while ((ident = ALooper_pollOnce(rhi_ok ? 0 : -1, nullptr, &events, (void**)&source)) >= 0)
		{
			if (source) source->process(m_app, source);
			if (!rhi_ok && m_native_window) { InitRHI(); rhi_ok = true; }
			if (m_destroy_requested) break;
		}
		if (rhi_ok && m_native_window && m_render)
		{
			m_render->OnUpdate(0.016f);
			Render::FrameContext ctx;
			ctx.viewport_width = m_width; ctx.viewport_height = m_height;
			m_render->OnPreRender(ctx);
			m_render->OnRender();
			m_render->OnPostRender(ctx);
			auto* cmd = RHIGetImmediateCommandList();
			m_viewport->Present(cmd, true, true);
		}
	}
	if (rhi_ok) { if (m_render) m_render->OnShutdown_Logic(); ShutdownRHI(); }
}

struct AAssetManager* AndroidWindow::GetAssetManager() CONST
{
	return (m_app && m_app->activity) ? m_app->activity->assetManager : nullptr;
}

// Android platform factory — platform_data is android_app* from SampleApp::SetPlatformData()
UniquePtr<PlatformWindow> CreatePlatformWindow(const String&, UInt32, UInt32, void* platform_data)
{
	auto* app = STATIC_CAST(platform_data, android_app);
	return app ? std::make_unique<AndroidWindow>(app) : nullptr;
}

MYRENDERER_END_NAMESPACE

#endif // PLATFORM_ANDROID
