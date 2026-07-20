// UE-style platform entry: the engine library provides android_main().
// Sample code only writes int main() — no Android-specific code needed.
#if PLATFORM_ANDROID

#include "Application/SampleApp.h"

extern int main(int argc, char** argv);

extern "C" void android_main(struct android_app* app)
{
	MXRender::Application::SampleApp::SetPlatformData(app);
	main(0, nullptr);
}

#endif // PLATFORM_ANDROID
