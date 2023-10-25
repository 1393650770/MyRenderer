#include "FilterSystem.h"

namespace MXRender
{
	FilterSystem::FilterSystem()
	{
	}

	FilterSystem::~FilterSystem()
	{
	}

	
	void FilterSystem::init(const std::string& keyword)
	{
		sentence_filter.insert(keyword);
	}

	std::vector<SentenceResult<char>>  FilterSystem::search(const std::string& str)
	{
		return sentence_filter.search_sentence(str);
	}

	bool FilterSystem::find(const std::string& str)
	{
		return sentence_filter.find(str);
	}

}

