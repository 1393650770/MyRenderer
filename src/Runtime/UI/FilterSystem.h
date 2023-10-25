#pragma once

#ifndef _FILTERSYSTEM_
#define _FILTERSYSTEM_



#include <memory>
#include <unordered_map>
#include <string>

namespace MXRender
{
	//构建多叉树结构的节点
	template<typename CharType>
	struct Node {
		std::basic_string<CharType> key;
		std::unordered_map<CharType, std::unique_ptr<Node<CharType >>> child;
	};

	template<typename CharType>
	class SentenceResult {
	public:
		std::basic_string<CharType> sentence;
		unsigned int start;
		unsigned int fin;
	public:
		explicit SentenceResult() {
			start = 0;
			fin = 0;
		}
		explicit SentenceResult(const std::basic_string<CharType>& sentence, unsigned int start, unsigned int fin) :
			sentence(sentence), start(start), fin(fin) {}
	};


	template<typename CharType>
	class SentenceFilter
	{
	public:
		SentenceFilter();
		virtual ~SentenceFilter();

		void insert(const std::basic_string<CharType>& str);

		bool find(const std::basic_string<CharType>& str);

		std::vector<SentenceResult<CharType>> search_sentence(const std::basic_string<CharType>& str);
	protected:
		std::unique_ptr<Node<CharType>> root;
	private:

	};

	template<typename CharType>
	std::vector<MXRender::SentenceResult<CharType>> MXRender::SentenceFilter<CharType>::search_sentence(const std::basic_string<CharType>& str)
	{
		std::vector<SentenceResult<CharType>> result;
		Node<CharType>* root = this->root.get();
		if (root == nullptr || str.empty())
		{
			return result;
		}

		Node<CharType>* current = root;
		unsigned int start = 0;
		unsigned int fin = 0;
		for (auto& ch : str)
		{
			if ((ch <= 0x20))
			{
				fin++;
				continue;
			}
			if (current->child.find(ch) == current->child.end())
			{
				current = root;
				start = fin + 1;
				fin = start;
			}
			else
			{
				current = current->child[ch].get();
				fin++;
				if (!current->key.empty())
				{
					result.emplace_back(current->key, start, fin);
					current = root;
					start = fin;
				}
			}
		}
		return result;

	}

	template<typename CharType>
	bool MXRender::SentenceFilter<CharType>::find(const std::basic_string<CharType>& str)
	{
		Node<CharType>* root = this->root.get();
		if (root == nullptr || str.empty())
		{
			return false;
		}

		Node<CharType>* current = root;
		for (auto& ch : str)
		{
			while ((ch <= 0x20))
			{
				continue;
			}
			if (current->child.find(ch) != current->child.end())
			{
				current = current->child[ch].get();
				if (!current->key.empty())
				{
					return true;
				}
			}
		}
		return false;
	}

	template<typename CharType>
	void MXRender::SentenceFilter<CharType>::insert(const std::basic_string<CharType>& str)
	{
		{
			Node<CharType>* root = this->root.get();
			if (root == nullptr || str.empty())
			{
				return;
			}

			Node<CharType>* current = this->root.get();
			for (auto& ch : str)
			{
				if (current->child.find(ch) == current->child.end())
				{
					current->child[ch] = std::make_unique<Node<CharType>>();
				}
				current = current->child[ch].get();
			}
			current->key = str;
		}
	}

	template<typename CharType>
	MXRender::SentenceFilter<CharType>::~SentenceFilter()
	{

	}

	template<typename CharType>
	MXRender::SentenceFilter<CharType>::SentenceFilter()
	{
		root = std::make_unique<Node<CharType>>();
	}

	class FilterSystem
	{
	public:
		FilterSystem();
		virtual ~FilterSystem();
		void init(const std::string& keyword );
		std::vector<SentenceResult<char>> search(const std::string& str);
		bool find(const std::string& str);
	protected:
		SentenceFilter<char> sentence_filter;

	};


}
#endif
