#pragma once
#ifndef _RMLUIDATAMODELMACROS_
#define _RMLUIDATAMODELMACROS_

/**
 * RmlUI DataModel binding macros — zero-dependency, no MetaParser required.
 *
 * == Quick start (in your .cpp) ==
 *
 *   auto ctor = ctx->CreateDataModel("hud");
 *   RMLUI_BIND_MODEL(ctor, &app,
 *       RMLUI_FIELD(m_hp,  "hp"),
 *       RMLUI_FIELD(m_score, "score"),
 *       RMLUI_FIELD(m_timer, "timer")
 *   );
 *
 * == MetaParser mode (future) ==
 *
 * When __REFLECTION_PARSER__ is defined, RMLUI_BIND_FIELD / RMLUI_BIND_FIELD_AS
 * emit annotation stubs that the MetaParser's rmlui_generator consumes to
 * auto-generate Bind*() functions.  Once the MetaParser environment is working
 * (libclang >= 20, all package include paths visible), the generated file at
 * _Generated/RmlUI/AllRmlDataModel.h replaces manual RMLUI_BIND_MODEL calls.
 */

// ── Always-available inline helpers (no MetaParser needed) ──────────────

// Bind a single field.  Works in any scope (needs Rml::DataModelConstructor in scope).
#define RMLUI_FIELD(data_member, display_name) \
	ctor.Bind(display_name, &data_member)

// Bind an event callback.
#define RMLUI_ACTION(method_name, display_name) \
	ctor.BindEventCallback(display_name, \
		[&](Rml::DataModelHandle h, Rml::Event& e, const Rml::VariantList& a) { \
			method_name(h, e, a); \
		})

// Bulk-bind multiple fields / actions in one statement.
// Usage: RMLUI_BIND_MODEL(ctor, data_ptr, RMLUI_FIELD(...), RMLUI_FIELD(...), ...)
#define RMLUI_BIND_MODEL(ctor_expr, data_ptr, ...) \
	do { \
		auto& ctor = (ctor_expr); \
		auto* data = (data_ptr); \
		(void)ctor; (void)data; \
		__VA_ARGS__; \
	} while(0)


// ── MetaParser annotation stubs ─────────────────────────────────────────
// Only active when MetaParser runs (__REFLECTION_PARSER__ defined).
// Emit named structs with "All" flag so shouldCompile() includes them.

#if defined(__REFLECTION_PARSER__)

	#define RMLUI_BIND_FIELD(T, field) \
		struct __attribute__((annotate("RmlBindField:" #T ":" #field ":" #field), annotate("All"))) \
		RmlBindField_##T##_##field {}

	#define RMLUI_BIND_FIELD_AS(T, field, name) \
		struct __attribute__((annotate("RmlBindField:" #T ":" #field ":" #name), annotate("All"))) \
		RmlBindField_##T##_##field {}

	#define RMLUI_BIND_ACTION(T, method, name) \
		struct __attribute__((annotate("RmlBindAction:" #T ":" #method ":" #name), annotate("All"))) \
		RmlBindAction_##T##_##method {}

#else

	#define RMLUI_BIND_FIELD(T, field)
	#define RMLUI_BIND_FIELD_AS(T, field, name)
	#define RMLUI_BIND_ACTION(T, method, name)

#endif // __REFLECTION_PARSER__

#endif // !_RMLUIDATAMODELMACROS_
