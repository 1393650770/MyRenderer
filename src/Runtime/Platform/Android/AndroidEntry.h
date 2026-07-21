#pragma once
// Include this header ONCE in each Sample's main .cpp to provide
// the android_main() entry point for NativeActivity.
// Do NOT add this file to the Runtime library — it must be compiled
// directly into the sample's .so so android_main() is found by the system.
#if defined(PLATFORM_ANDROID) || defined(__ANDROID__)
#include "Application/SampleApp.h"
#include <android_native_app_glue.h>

extern "C" __attribute__((visibility("default"))) void android_main(struct android_app* app)
{
	MXRender::Application::SampleApp::SetPlatformData(app);
	// main() is defined in the same translation unit (the Sample's .cpp)
	extern int main();
	main();
}
#endif // PLATFORM_ANDROID || __ANDROID__
