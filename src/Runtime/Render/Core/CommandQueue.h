#pragma once
#ifndef _RENDER_COMMANDQUEUE_
#define _RENDER_COMMANDQUEUE_

#include "Core/ConstDefine.h"
#include <functional>
#include <mutex>
#include <condition_variable>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

// Thread-safe command queue with bypass mode.
// Bypass=true: commands execute immediately on the calling thread (single-threaded).
// Bypass=false: commands are enqueued for consumption by a dedicated thread.

using RenderCommand = std::function<void()>;

MYRENDERER_BEGIN_CLASS(CommandQueue)
public:
	CommandQueue() MYDEFAULT;
	~CommandQueue() MYDEFAULT;

	void METHOD(SetBypass)(Bool in_bypass) { bypass = in_bypass; }
	Bool METHOD(IsBypass)() CONST { return bypass; }

	void METHOD(Enqueue)(RenderCommand cmd)
	{
		if (bypass) { cmd(); return; }
		{
			std::lock_guard<std::mutex> lock(mtx);
			commands.push_back(std::move(cmd));
		}
		cv.notify_one();
	}

	void METHOD(Flush)()
	{
		Vector<RenderCommand> batch;
		{
			std::lock_guard<std::mutex> lock(mtx);
			batch.swap(commands);
		}
		for (auto& cmd : batch) cmd();
	}

	// Block until commands are available, then process them.
	// Returns false if the queue has been shut down.
	Bool METHOD(WaitAndFlush)()
	{
		if (bypass) return true;
		Vector<RenderCommand> batch;
		{
			std::unique_lock<std::mutex> lock(mtx);
			cv.wait(lock, [this]{ return !commands.empty() || !running; });
			if (!running && commands.empty()) return false;
			batch.swap(commands);
		}
		for (auto& cmd : batch) cmd();
		return true;
	}

	// Signal shutdown — wakes the consumer so it can exit.
	void METHOD(Shutdown)()
	{
		{
			std::lock_guard<std::mutex> lock(mtx);
			running = false;
		}
		cv.notify_one();
	}

	Bool METHOD(IsRunning)() CONST { return running; }

private:
	mutable std::mutex mtx;
	std::condition_variable cv;
	Vector<RenderCommand> commands;
	Bool bypass = true;
	Bool running = true;
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#define ENQUEUE_RENDER_COMMAND(queue, body) \
	(queue).Enqueue([&]() body)

#endif
