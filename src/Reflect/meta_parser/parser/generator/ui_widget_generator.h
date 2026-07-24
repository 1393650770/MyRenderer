#pragma once
#include "generator/generator.h"

namespace Generator
{
	/**
	 * Generator for UIWidgetBindingTraits<T> specializations.
	 *
	 * Scans field-level UI_BIND(Enable, ...) annotations (UE-style metadata
	 * system).  No stub structs — annotations go directly on the field.
	 *
	 * Per-file:  src/_Generated/RmlUI/Filename.UIBinding.Gen.h
	 * Aggregate: src/_Generated/RmlUI/AllUIWidgetBindings.h
	 */
	class UIWidgetGenerator : public GeneratorInterface
	{
	public:
		UIWidgetGenerator() = delete;
		UIWidgetGenerator(std::string source_directory,
			std::function<std::string(std::string)> get_include_function);
		virtual int  generate(std::string path, SchemaMoudle schema) override;
		virtual void finish() override;
		virtual ~UIWidgetGenerator() override;

	protected:
		virtual void        prepareStatus(std::string path) override;
		virtual std::string processFileName(std::string path) override;

	private:
		std::vector<std::string> m_head_file_list;
	};
} // namespace Generator
