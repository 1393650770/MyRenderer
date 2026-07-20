#include "AndroidApp.h"

#if PLATFORM_ANDROID

#include "RHI/RenderRHI.h"
#include "RHI/RenderViewport.h"
#include "RHI/RenderCommandList.h"
#include "Tool/ShaderLibrary.h"
#include <android/log.h>
#include <android/input.h>

#define LOG_TAG "MXRender"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)

AndroidApp* AndroidApp::s_instance = nullptr;

AndroidApp::AndroidApp(struct android_app* in_app)
	: app(in_app)
{
	s_instance = this;
	app->userData = this;
	app->onAppCmd = &AndroidApp::OnAppCmd;
	app->onInputEvent = &AndroidApp::OnInputEvent;
}

AndroidApp::~AndroidApp()
{
	if (s_instance == this)
		s_instance = nullptr;
}

void AndroidApp::OnAppCmd(struct android_app* in_app, int32_t cmd)
{
	AndroidApp* self = STATIC_CAST(in_app->userData, AndroidApp);
	if (self)
		self->HandleCmd(cmd);
}

void AndroidApp::HandleCmd(int32_t cmd)
{
	switch (cmd)
	{
	case APP_CMD_INIT_WINDOW:
		native_window = app->window;
		LOGI("APP_CMD_INIT_WINDOW: window=%p", native_window);
		break;

	case APP_CMD_TERM_WINDOW:
		LOGI("APP_CMD_TERM_WINDOW");
		if (on_shutdown) on_shutdown();
		ShutdownRHI();
		native_window = nullptr;
		break;

	case APP_CMD_GAINED_FOCUS:
		has_focus = true;
		break;

	case APP_CMD_LOST_FOCUS:
		has_focus = false;
		break;

	case APP_CMD_DESTROY:
		LOGI("APP_CMD_DESTROY");
		is_destroy_requested = true;
		break;

	case APP_CMD_WINDOW_RESIZED:
		LOGI("APP_CMD_WINDOW_RESIZED");
		break;

	default:
		break;
	}
}

int32_t AndroidApp::OnInputEvent(struct android_app* in_app, AInputEvent* event)
{
	AndroidApp* self = STATIC_CAST(in_app->userData, AndroidApp);
	if (self)
		self->HandleMotionEvent(event);
	return 0; // 0 = not handled, let Android process it
}

void AndroidApp::HandleMotionEvent(AInputEvent* event)
{
	if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_MOTION)
		return;

	Int action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
	Int pointer_count = AMotionEvent_getPointerCount(event);
	if (pointer_count > TouchState::kMaxPointers)
		pointer_count = TouchState::kMaxPointers;

	touch_state.pointer_count = pointer_count;
	for (Int i = 0; i < pointer_count; ++i)
	{
		touch_state.pointers[i].x = AMotionEvent_getX(event, i);
		touch_state.pointers[i].y = AMotionEvent_getY(event, i);
		touch_state.pointers[i].id = AMotionEvent_getPointerId(event, i);
		touch_state.pointers[i].active = true;
	}

	// Clear remaining pointer slots
	for (Int i = pointer_count; i < TouchState::kMaxPointers; ++i)
		touch_state.pointers[i].active = false;

	// Pinch distance between first two active pointers
	if (pointer_count >= 2)
	{
		Float32 dx = touch_state.pointers[0].x - touch_state.pointers[1].x;
		Float32 dy = touch_state.pointers[0].y - touch_state.pointers[1].y;
		touch_state.pinch_distance = sqrtf(dx * dx + dy * dy);
	}
	else
	{
		touch_state.pinch_distance = 0.0f;
	}

	// On ACTION_UP or ACTION_CANCEL, clear pointers
	if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_CANCEL)
	{
		touch_state.pointer_count = 0;
		for (Int i = 0; i < TouchState::kMaxPointers; ++i)
			touch_state.pointers[i].active = false;
		touch_state.pinch_distance = 0.0f;
	}
}

void AndroidApp::InitRHI()
{
	if (native_window == nullptr)
	{
		LOGE("InitRHI: null native window");
		return;
	}

	width = ANativeWindow_getWidth(STATIC_CAST(native_window, ANativeWindow));
	height = ANativeWindow_getHeight(STATIC_CAST(native_window, ANativeWindow));
	LOGI("InitRHI: %dx%d", width, height);

	RHIInit();

	// Register asset manager for ShaderLibrary APK reads
	if (app->activity && app->activity->assetManager)
		MXRender::Tool::ShaderLibrary::SetAssetManager(app->activity->assetManager);

	viewport = RHICreateViewport(native_window, width, height, false);

	if (on_init)
		on_init();
}

void AndroidApp::ShutdownRHI()
{
	LOGI("ShutdownRHI");

	if (viewport)
	{
		delete viewport;
		viewport = nullptr;
	}

	RHIShutdown();
}

void AndroidApp::Run(std::function<void()> in_on_init, std::function<void()> in_on_frame,
                     std::function<void()> in_on_shutdown)
{
	on_init = std::move(in_on_init);
	on_frame = std::move(in_on_frame);
	on_shutdown = std::move(in_on_shutdown);

	// Android NDK glue event loop.
	// Phase 1: poll for events, render when window is ready.
	Bool rhi_initialized = false;

	while (!is_destroy_requested)
	{
		// Read all pending events
		int ident;
		int events;
		struct android_poll_source* source;

		while ((ident = ALooper_pollOnce(rhi_initialized ? 0 : -1, nullptr, &events, (void**)&source)) >= 0)
		{
			if (source != nullptr)
				source->process(app, source);

			// Check if window became available after processing events
			if (!rhi_initialized && native_window != nullptr)
			{
				InitRHI();
				rhi_initialized = true;
			}

			if (is_destroy_requested)
				break;
		}

		// Render frame if RHI is initialized and window is ready
		if (rhi_initialized && native_window != nullptr && has_focus && on_frame)
		{
			on_frame();
		}
	}

	// Ensure cleanup
	if (rhi_initialized)
	{
		if (on_shutdown) on_shutdown();
		ShutdownRHI();
	}
}

struct AAssetManager* AndroidApp::GetAssetManager() CONST
{
	if (app && app->activity)
		return app->activity->assetManager;
	return nullptr;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif // PLATFORM_ANDROID
