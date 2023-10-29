#include "FilterSystem.h"

namespace MXRender
{
	FilterSystem::FilterSystem()
	{
	}

	FilterSystem::~FilterSystem()
	{
	}

	
	void FilterSystem::insert(const std::string& keyword)
	{
		sentence_filter.insert(keyword);
	}

	std::vector<SentenceResult<char>>  FilterSystem::search(const std::string& str)
	{
		return sentence_filter.search_sentence(str);
	}

	std::string FilterSystem::replace(const std::string& str, char replace_char)
	{
		auto replace_result = search(str);
		std::string ret = str;
		for (auto& it : replace_result)
		{
			for (int i = it.start; i < it.fin; i++)
			{
				ret[i] = replace_char;
			}
		}
		return ret;
	}

	bool FilterSystem::find(const std::string& str)
	{
		return sentence_filter.find(str);
	}

}

