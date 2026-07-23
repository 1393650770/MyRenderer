#include "common/precompiled.h"

#include "generator/rmlui_generator.h"

#include "language_types/class.h"
#include "template_manager/template_manager.h"
#include "meta/meta_utils.h"

#include <map>
#include <set>
#include <iostream>

namespace Generator
{
	RmluiGenerator::RmluiGenerator(std::string                             source_directory,
	                               std::function<std::string(std::string)> get_include_function) :
		GeneratorInterface(source_directory + "/_Generated/RmlUI", source_directory, get_include_function)
	{
		prepareStatus(m_out_path);
	}

	void RmluiGenerator::prepareStatus(std::string path)
	{
		GeneratorInterface::prepareStatus(path);
		TemplateManager::getInstance()->loadTemplates(m_root_path, "rmlDataModelBinding");
		std::cout << "[RmluiGenerator] Template loaded, output: " << m_out_path << std::endl;
	}

	std::string RmluiGenerator::processFileName(std::string path)
	{
		// All bindings go into a single aggregate file
		(void)path;
		return m_out_path + "/AllRmlDataModel.h";
	}

	int RmluiGenerator::generate(std::string path, SchemaMoudle schema)
	{
		// Scan all classes for RmlBindField / RmlBindAction annotations
		for (auto class_temp : schema.classes)
		{
			if (!class_temp->shouldCompileFields())
				continue;

			// Check if the class has RmlUI annotation properties
			std::string class_name = class_temp->getClassName();
			std::string qualified_name = class_temp->m_name; // fully qualified

			auto& model = m_models[class_name];
			model.class_name = class_name;
			if (model.model_name.empty())
				model.model_name = class_name;

			// Scan fields for annotation patterns
			// MetaInfo stores key-value pairs from __attribute__((annotate(...)))
			// We look for annotations like "RmlBindField:Type:Field:Display"
			for (auto field : class_temp->m_fields)
			{
				if (!field->shouldCompile())
					continue;

				// Check if this field has an annotation resembling our format
				const auto& props = field->getMetaData().getProperties();
				for (const auto& kv : props)
				{
					std::string key = kv.first;
					// Look for annotations with "RmlBindField" or "RmlBindAction" prefix
					// Format: "RmlBindField:FieldName:DisplayName" or just field-level annotations
					if (key.find("RmlBindField") == 0 || key.find("RmlBindAction") == 0)
					{
						std::cout << "  [RmluiGenerator] " << class_name
							<< " annotation: " << key << std::endl;
					}
				}
			}
		}

		// Check stub structs with annotations
		// The stub struct pattern creates empty structs like:
		//   struct __attribute__((annotate("RmlBindField:PlayerHUD:hp:hp"))) RmlBindField_PlayerHUD_hp {};
		// These are detected as classes in the schema.
		for (auto class_temp : schema.classes)
		{
			std::string class_name = class_temp->getClassName();
			// Check if class name starts with "RmlBindField_" or "RmlBindAction_"
			if (class_name.find("RmlBindField_") == 0)
			{
				// Parse: RmlBindField_TypeName_field
				std::string suffix = class_name.substr(14); // after "RmlBindField_"
				size_t pos = suffix.rfind('_');
				if (pos != std::string::npos)
				{
					std::string type_name = suffix.substr(0, pos);
					std::string field_name = suffix.substr(pos + 1);

					// Get the annotation from the class's meta data
					const auto& props = class_temp->getMetaData().getProperties();
					std::string display_name = field_name; // default
					for (const auto& kv : props)
					{
						if (kv.first.find("RmlBindField") == 0)
						{
							// The value is the full string: "RmlBindField:Type:Field:Display"
							// Parse it
							std::string val = kv.second;
							if (!val.empty())
							{
								// Split by ':'
								std::vector<std::string> parts;
								size_t start = 0;
								for (size_t i = 0; i < val.size(); i++)
								{
									if (val[i] == ':')
									{
										parts.push_back(val.substr(start, i - start));
										start = i + 1;
									}
								}
								parts.push_back(val.substr(start));
								if (parts.size() >= 3)
								{
									type_name = parts[0];
									field_name = parts[1];
									display_name = parts[2];
								// Strip quotes from libclang annotation display name
								if (display_name.size() >= 2 && display_name.front() == '"' && display_name.back() == '"')
									display_name = display_name.substr(1, display_name.size() - 2);
								}
							}
						}
					}

					auto& model = m_models[type_name];
					model.class_name = type_name;
					if (model.model_name.empty())
						model.model_name = type_name;
					model.fields.push_back({ field_name, display_name });

					std::cout << "  [RmluiGenerator] BindField: " << type_name
						<< "::" << field_name << " -> " << display_name << std::endl;
				}
			}
			else if (class_name.find("RmlBindAction_") == 0)
			{
				std::string suffix = class_name.substr(15); // after "RmlBindAction_"
				size_t pos = suffix.rfind('_');
				if (pos != std::string::npos)
				{
					std::string type_name = suffix.substr(0, pos);
					std::string method_name = suffix.substr(pos + 1);

					const auto& props = class_temp->getMetaData().getProperties();
					std::string display_name = method_name;
					for (const auto& kv : props)
					{
						if (kv.first.find("RmlBindAction") == 0)
						{
							std::string val = kv.second;
							if (!val.empty())
							{
								std::vector<std::string> parts;
								size_t start = 0;
								for (size_t i = 0; i < val.size(); i++)
								{
									if (val[i] == ':')
									{
										parts.push_back(val.substr(start, i - start));
										start = i + 1;
									}
								}
								parts.push_back(val.substr(start));
								if (parts.size() >= 3)
								{
									type_name = parts[0];
									method_name = parts[1];
									display_name = parts[2];
								// Strip quotes from libclang annotation display name
								if (display_name.size() >= 2 && display_name.front() == '"' && display_name.back() == '"')
									display_name = display_name.substr(1, display_name.size() - 2);
								}
							}
						}
					}

					auto& model = m_models[type_name];
					model.class_name = type_name;
					if (model.model_name.empty())
						model.model_name = type_name;
					model.actions.push_back({ method_name, display_name });

					std::cout << "  [RmluiGenerator] BindAction: " << type_name
						<< "::" << method_name << " -> " << display_name << std::endl;
				}
			}
		}

		return 0;
	}

	void RmluiGenerator::finish()
	{
		if (m_models.empty())
		{
			std::cout << "[RmluiGenerator] No RmlUI bindings found. Skipping output." << std::endl;
			return;
		}

		Mustache::data mustache_data;
		Mustache::data models_list(Mustache::data::type::list);

		for (const auto& kv : m_models)
		{
			const auto& model = kv.second;
			if (model.fields.empty() && model.actions.empty())
				continue;

			Mustache::data model_def;
			model_def.set("model_name", model.model_name);
			model_def.set("class_name", model.class_name);

			Mustache::data field_list(Mustache::data::type::list);
			for (const auto& f : model.fields)
			{
				Mustache::data fd;
				fd.set("field_name", f.first);
				fd.set("display_name", f.second);
				field_list.push_back(fd);
			}
			model_def.set("fields", field_list);

			Mustache::data action_list(Mustache::data::type::list);
			for (const auto& a : model.actions)
			{
				Mustache::data ad;
				ad.set("method_name", a.first);
				ad.set("display_name", a.second);
				action_list.push_back(ad);
			}
			model_def.set("actions", action_list);

			models_list.push_back(model_def);
		}

		mustache_data.set("models", models_list);

		std::string render_string =
			TemplateManager::getInstance()->renderByTemplate("rmlDataModelBinding", mustache_data);

		std::string file_path = processFileName("");
		Utils::saveFile(render_string, file_path);
		std::cout << "[RmluiGenerator] Generated " << file_path
			<< " with " << m_models.size() << " models." << std::endl;
	}

	RmluiGenerator::~RmluiGenerator() {}
} // namespace Generator
