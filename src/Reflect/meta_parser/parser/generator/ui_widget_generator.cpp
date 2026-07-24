#include "common/precompiled.h"

#include "generator/ui_widget_generator.h"

#include "language_types/class.h"
#include "template_manager/template_manager.h"
#include "meta/meta_utils.h"

#include <map>
#include <iostream>

namespace Generator
{
	UIWidgetGenerator::UIWidgetGenerator(std::string                             source_directory,
		std::function<std::string(std::string)> get_include_function) :
		GeneratorInterface(source_directory + "/_Generated/RmlUI", source_directory, get_include_function)
	{
		prepareStatus(m_out_path);
	}

	void UIWidgetGenerator::prepareStatus(std::string path)
	{
		GeneratorInterface::prepareStatus(path);
		TemplateManager::getInstance()->loadTemplates(m_root_path, "uiWidgetBindingFile");
		TemplateManager::getInstance()->loadTemplates(m_root_path, "allUIWidgetBindingFile");
		std::cout << "[UIWidgetGenerator] Templates loaded, output: " << m_out_path << std::endl;
	}

	std::string UIWidgetGenerator::processFileName(std::string path)
	{
		auto stem = fs::path(path).filename().replace_extension("UIBinding.Gen.h").string();
		return m_out_path + "/" + stem;
	}

	int UIWidgetGenerator::generate(std::string path, SchemaMoudle schema)
	{
		// Field-level binding data — collected from separate-key annotations:
		//   UI_BIND(Enable) → "UIBind": "Enable"
		//   UI_AS(hp)       → "FIELD_AS": "hp"
		//   UI_TWOWAY        → "TWO_WAY": ""
		//   UI_EVENT(m, n)   → "UIBind": "Enable", "EVENT": "m", "FIELD_AS": "n"
		struct FieldBind {
			std::string field_name;
			std::string rml_name;
			bool        is_twoway = false;
			std::string event_method;
		};
		std::map<std::string, std::vector<FieldBind>> class_fields;

		for (auto class_temp : schema.classes)
		{
			if (class_temp->m_fields.empty()) continue;
			std::string class_name = class_temp->getClassName();

			for (auto field : class_temp->m_fields)
			{
				const auto& props = field->getMetaData().getProperties();
				if (props.find("UIBind") == props.end()) continue;

				FieldBind fb;
				fb.field_name = field->m_name;
				fb.rml_name   = field->m_name;

				// Parse all props — keys may be "KEY=VALUE" (from annotate("KEY=VALUE"))
				// or plain "FLAG" (from annotate("FLAG")).
				for (const auto& kv : props) {
					const std::string& raw = kv.first;
					if (raw == "UIBind") continue;

					// Split "KEY=VALUE" → key, val; or "FLAG" → flag
					auto eq = raw.find('=');
					if (eq != std::string::npos) {
						std::string k = raw.substr(0, eq);
						std::string v = raw.substr(eq + 1);
						if (k == "FIELD_AS") fb.rml_name = v;
						else if (k == "EVENT") fb.event_method = v;
					} else {
						if (raw == "TWO_WAY") fb.is_twoway = true;
					}
				}

				class_fields[class_name].push_back(fb);

				std::cout << "  [UIWidgetGenerator] " << class_name
					<< "::" << fb.field_name << " -> \"" << fb.rml_name << "\""
					<< (fb.is_twoway ? " (TwoWay)" : "")
					<< (!fb.event_method.empty() ? (" (Event:" + fb.event_method + ")") : "")
					<< std::endl;
			}
		}

		if (class_fields.empty()) return 0;

		Mustache::data mustache_data;
		Mustache::data class_defines(Mustache::data::type::list);
		Mustache::data include_headfiles(Mustache::data::type::list);

		include_headfiles.push_back(
			Mustache::data("headfile_name",
				Utils::makeRelativePath(m_root_path + "/_Generated/RmlUI", path).string()));

		for (auto& kv : class_fields)
		{
			const std::string& class_name = kv.first;
			auto& fields = kv.second;

			Mustache::data traits_def;
			traits_def.set("class_name", class_name);

			Mustache::data field_list(Mustache::data::type::list);
			Mustache::data twoway_list(Mustache::data::type::list);
			Mustache::data action_list(Mustache::data::type::list);

			for (auto& fb : fields)
			{
				if (!fb.event_method.empty())
				{
					Mustache::data ad;
					ad.set("method_name", fb.event_method);
					ad.set("display_name", fb.rml_name);
					action_list.push_back(ad);
				}
				else if (fb.is_twoway)
				{
					Mustache::data fd;
					fd.set("field_name", fb.field_name);
					fd.set("display_name", fb.rml_name);
					twoway_list.push_back(fd);
				}
				else
				{
					Mustache::data fd;
					fd.set("field_name", fb.field_name);
					fd.set("display_name", fb.rml_name);
					field_list.push_back(fd);
				}
			}

			traits_def.set("fields", field_list);
			traits_def.set("twoway_fields", twoway_list);
			traits_def.set("actions", action_list);
			class_defines.push_back(traits_def);
		}

		mustache_data.set("class_defines", class_defines);
		mustache_data.set("include_headfiles", include_headfiles);

		std::string file_path = processFileName(path);
		std::string render_string =
			TemplateManager::getInstance()->renderByTemplate("uiWidgetBindingFile", mustache_data);
		Utils::saveFile(render_string, file_path);

		m_head_file_list.emplace_back(
			Utils::makeRelativePath(m_root_path + "/_Generated/RmlUI", file_path).string());

		std::cout << "  [UIWidgetGenerator] Generated " << file_path
			<< " with " << class_fields.size() << " traits." << std::endl;

		return 0;
	}

	void UIWidgetGenerator::finish()
	{
		if (m_head_file_list.empty())
		{
			std::cout << "[UIWidgetGenerator] No UIWidget bindings found. Skipping aggregate." << std::endl;
			return;
		}

		Mustache::data mustache_data;
		Mustache::data include_headfiles(Mustache::data::type::list);

		for (auto& head_file : m_head_file_list)
			include_headfiles.push_back(Mustache::data("headfile_name", head_file));
		mustache_data.set("include_headfiles", include_headfiles);

		std::string render_string =
			TemplateManager::getInstance()->renderByTemplate("allUIWidgetBindingFile", mustache_data);
		Utils::saveFile(render_string, m_out_path + "/AllUIWidgetBindings.h");

		std::cout << "[UIWidgetGenerator] Generated aggregate "
			<< m_out_path << "/AllUIWidgetBindings.h with "
			<< m_head_file_list.size() << " files." << std::endl;
	}

	UIWidgetGenerator::~UIWidgetGenerator() {}
} // namespace Generator
