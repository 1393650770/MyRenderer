#pragma once
#ifndef _RENDERGRAPH_AUTOSAVESERVICE_
#define _RENDERGRAPH_AUTOSAVESERVICE_

#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

MYRENDERER_BEGIN_CLASS(AutoSaveService)
public:
	AutoSaveService() MYDEFAULT;
	~AutoSaveService() MYDEFAULT;

	void METHOD(Init)(CONST String& auto_save_path);

	void METHOD(MarkDirty)();

	void METHOD(Tick)(Float32 delta_seconds);

	Bool METHOD(HasAutoSave)() CONST;
	Bool METHOD(RecoverAutoSave)(struct Render::RenderGraphDefinition& out_def) CONST;

	void METHOD(Clear)();

private:
	String m_auto_save_path;
	String m_temp_path;
	Float32 m_interval = 30.0f;
	Float32 m_timer = 0.0f;
	Bool m_dirty = false;
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
