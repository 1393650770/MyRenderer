#pragma once
#ifndef _UIWIDGETMACROS_
#define _UIWIDGETMACROS_

// Simplest possible approach — same pattern as META(...) in ConstDefine.h
// UI_BIND(Enable)          → annotate("UIBind:Enable")
// UI_BIND(Enable,FOO=bar)  → annotate("UIBind:Enable, FOO=bar") → extractProperties splits by comma

#if defined(__REFLECTION_PARSER__)
	#define UI_BIND(...) __attribute__((annotate("UIBind:" #__VA_ARGS__)))
#else
	#define UI_BIND(...)
#endif

#endif
