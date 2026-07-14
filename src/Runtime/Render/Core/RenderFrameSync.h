#pragma once
#ifndef _RENDER_FRAMESYNC_
#define _RENDER_FRAMESYNC_

#include "Core/ConstDefine.h"
#include "Core/ConstGlobals.h"
#include "Render/Core/RenderFrameData.h"
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Viewport;
MYRENDERER_END_NAMESPACE

class RenderInterface;

MYRENDERER_BEGIN_NAMESPACE(Render)

//  Triple-buffered frame synchronizer for Logic -> Render -> RHI pipeline
// Uses condition_variable initially; upgrade path: lock-free ring buffer
MYRENDERER_BEGIN_CLASS(FrameSynchronizer)
public:
	static constexpr UInt32 kMaxFramesInFlight = 3;

	FrameSynchronizer();
	~FrameSynchronizer();
	FrameSynchronizer(CONST FrameSynchronizer&) = delete;
	FrameSynchronizer& operator=(CONST FrameSynchronizer&) = delete;

	// === Logic Thread API ===
	// Block until a free FrameContext slot is available. Returns a writable FrameContext.
	FrameContext* METHOD(AcquireWriteSlot)();

	// Mark current write slot as "Ready" for the Render thread.
	void METHOD(SignalFrameReady)();

	// Block until a frame has completed (Render done + RHI present done).
	// Returns the completed FrameContext.
	FrameContext* METHOD(WaitFrameComplete)();

	// === Render Thread API ===
	// Block until a frame is ready for rendering. Returns read-only FrameContext.
	FrameContext* METHOD(WaitFrameReady)();

	// Signal that rendering is done (commands recorded, swapped to RHI).
	void METHOD(SignalRenderDone)();

	// === Render thread lifecycle ===
	Bool METHOD(IsRenderRunning)() CONST;
	void METHOD(StartRenderThread)(RenderInterface* render, RHI::Viewport* viewport);
	void METHOD(StopRenderThread)();
	void METHOD(JoinRenderThread)();

	// === Query ===
	UInt32 METHOD(GetFramesInFlight)() CONST { return frames_in_flight; }

private:
	void RenderThreadMain(RenderInterface* render, RHI::Viewport* viewport);

	// Triple buffer of frame contexts
	FrameContext contexts[kMaxFramesInFlight];

	// Per-slot state machine
	enum class SlotState : UInt8
	{
		Free,              // Available for Logic to write
		LogicWriting,      // Logic is filling this slot
		Ready,             // Logic done; Render can consume
		RenderProcessing,  // Render is using this slot
		Done               // Render+RHI done; Logic can call PostFrame
	};
	SlotState slot_states[kMaxFramesInFlight] = {};

	// Indices into ring buffer
	UInt32 write_index = 0;      // Next slot for Logic to write
	UInt32 render_index = 0;     // Next slot for Render to read
	UInt32 complete_index = 0;   // Next slot for Logic's PostFrame

	UInt32 frames_in_flight = 0;

	// Synchronization primitives
	std::mutex mtx;
	std::condition_variable cv_logic;      // Logic waits for free slot
	std::condition_variable cv_render;     // Render waits for ready frame
	std::condition_variable cv_complete;   // Logic waits for completion

	// Render thread handle
	std::thread render_thread;
	std::atomic<Bool> render_running{false};

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
