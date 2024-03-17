
#pragma once
#ifndef _RENDERGRAPHPANNEL_
#define _RENDERGRAPHPANNEL_
#include "BasePanel.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderGraphPanel,public BasePanel)


#pragma region METHOD
public:
	VIRTUAL ~RenderGraphPanel() MYDEFAULT;
	RenderGraphPanel() MYDEFAULT;
	RenderGraphPanel(CONST String& in_name = "RenderGraphPanel", Bool in_show = true);
	RenderGraphPanel(CONST RenderGraphPanel& other) MYDELETE;
	RenderGraphPanel(RenderGraphPanel&& other) MYDELETE;
	RenderGraphPanel& operator=(CONST RenderGraphPanel& other) MYDELETE;
	RenderGraphPanel& operator=(RenderGraphPanel&& other) MYDELETE;

	static CONST String METHOD(GetTypeName)();

	VIRTUAL void METHOD(Init)() OVERRIDE FINAL;
	VIRTUAL void METHOD(Update)() OVERRIDE FINAL;
	VIRTUAL void METHOD(Draw)() OVERRIDE FINAL;
	VIRTUAL void METHOD(Release)() OVERRIDE FINAL;
protected:
private:

#pragma endregion

#pragma region MEMBER
public:
	Bool is_show = true;
protected:
	String name = "";

private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_RENDERGRAPHPANNEL_