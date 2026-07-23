#pragma once
#include "generator/generator.h"

namespace Generator
{
	/**
	 * Generator for RmlUI DataModel bindings.
	 *
	 * Scans all classes for "RmlBindField:Type:Field:DisplayName" and
	 * "RmlBindAction:Type:Method:DisplayName" annotations, groups them by
	 * class name, and generates inline Bind*() functions.
	 *
	 * Output: src/_Generated/RmlUI/AllRmlDataModel.h
	 */
	class RmluiGenerator : public GeneratorInterface
	{
	public:
		RmluiGenerator() = delete;
		RmluiGenerator(std::string source_directory,
		               std::function<std::string(std::string)> get_include_function);
		virtual int  generate(std::string path, SchemaMoudle schema) override;
		virtual void finish() override;
		virtual ~RmluiGenerator() override;

	protected:
		virtual void        prepareStatus(std::string path) override;
		virtual std::string processFileName(std::string path) override;

	private:
		// Accumulated data models grouped by class name
		struct DataModel {
			std::string class_name;
			std::string model_name; // display name for the model
			std::vector<std::pair<std::string, std::string>> fields; // field_name, display_name
			std::vector<std::pair<std::string, std::string>> actions; // method_name, display_name
		};
		std::map<std::string, DataModel> m_models; // keyed by class_name
	};
} // namespace Generator
