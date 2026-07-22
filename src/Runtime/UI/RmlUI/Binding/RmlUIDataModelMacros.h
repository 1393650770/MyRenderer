#pragma once
#ifndef _RMLUIDATAMODELMACROS_
#define _RMLUIDATAMODELMACROS_

/**
 * RmlUI DataModel binding macros.
 *
 * These macros define a code-generation annotation system that works with
 * the MetaParser to auto-generate Rml::DataModelConstructor Bind/BindEventCallback
 * calls for C++ structs.
 *
 * Usage:
 *   In a .h file, declare your data model struct and annotate it:
 *
 *   struct PlayerHUD {
 *       int hp = 100;
 *       String name;
 *       void on_click(Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) {}
 *   };
 *
 *   // Define bindings (after the struct definition)
 *   RMLUI_BIND_FIELD(PlayerHUD, hp);
 *   RMLUI_BIND_FIELD_AS(PlayerHUD, name, "display_name");
 *   RMLUI_BIND_ACTION(PlayerHUD, on_click, "click");
 *
 *   Then include the generated file and call:
 *   MXRender::UI::RmlUI::Generated::BindPlayerHUD(ctor, &player);
 *
 * When __REFLECTION_PARSER__ is defined (MetaParser mode), these macros emit
 * __attribute__((annotate(...))) on stub structs that the parser recognizes.
 */

#if defined(__REFLECTION_PARSER__)
	// MetaParser mode: emit annotation stubs

	#define RMLUI_INTERNAL_ANNOTATE(annotation) \
		struct __attribute__((annotate(annotation)))

	#define RMLUI_BIND_FIELD(T, field) \
		RMLUI_INTERNAL_ANNOTATE("RmlBindField:" #T ":" #field ":" #field) {}

	#define RMLUI_BIND_FIELD_AS(T, field, name) \
		RMLUI_INTERNAL_ANNOTATE("RmlBindField:" #T ":" #field ":" #name) {}

	#define RMLUI_BIND_ACTION(T, method, name) \
		RMLUI_INTERNAL_ANNOTATE("RmlBindAction:" #T ":" #method ":" #name) {}

#else
	// Normal compilation: macros are no-ops. The generated file provides the actual binding code.

	#define RMLUI_BIND_FIELD(T, field)
	#define RMLUI_BIND_FIELD_AS(T, field, name)
	#define RMLUI_BIND_ACTION(T, method, name)

#endif // __REFLECTION_PARSER__

/**
 * Convenience macro: manually bind a field without code generation.
 * Use this when you don't want to run MetaParser.
 *
 * Usage:
 *   RMLUI_BIND(ctor, &player.hp, "hp");
 */
#define RMLUI_BIND(ctor, ptr, name) \
	ctor.Bind(name, ptr)

/**
 * Convenience macro: manually bind an event callback without code generation.
 *
 * Usage:
 *   RMLUI_BIND_EVENT(ctor, "click", [&](auto h, auto& e, auto& a) { ... });
 */
#define RMLUI_BIND_EVENT(ctor, name, lambda) \
	ctor.BindEventCallback(name, lambda)

#endif // !_RMLUIDATAMODELMACROS_
