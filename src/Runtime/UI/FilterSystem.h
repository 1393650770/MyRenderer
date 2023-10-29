#pragma once

#ifndef _FILTERSYSTEM_
#define _FILTERSYSTEM_



#include <memory>
#include <unordered_map>
#include <string>
#include <stack>
#include <unordered_set>
#include <type_traits>

namespace MXRender
{
	//构建多叉树结构的节点
	template<typename CharType>
	struct Node {
		std::basic_string<CharType> key;
		std::unordered_map<CharType, std::shared_ptr<Node<CharType >>> child;
		std::vector<Node<CharType>*> nfa_next_state;
	};
	template<typename CharType>
	struct Nfa
	{
		Node<CharType>* start;
		Node<CharType>* end;
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
		std::vector<std::unique_ptr<Node<CharType>>> ends;
		std::string special_char = "|*()&";
		bool check_char(char ch);
		void generate_nfa(std::stack<Nfa<CharType>>& nfas, std::stack<char>& ops);
		bool search_sentence_recursion( std::vector < SentenceResult<CharType>>& ret, Node<CharType>* node, std::unordered_set<Node<CharType>*>& visited_set, const std::basic_string<CharType>& str, unsigned int& start, unsigned int& end,int cur, int depth);
	private:
	};

	template<typename CharType>
	void MXRender::SentenceFilter<CharType>::generate_nfa(std::stack<Nfa<CharType>>& nfas, std::stack<char>& ops)
	{
		if (ops.top() == '|')
		{
			ops.pop();
			if (nfas.size() >= 2)
			{
				Nfa<CharType> second = nfas.top();
				nfas.pop();
				Nfa<CharType> first = nfas.top();
				nfas.pop();

				Nfa<CharType> nfa;
				for (auto& it : second.start->child)
				{
					first.start->child[it.first] = std::shared_ptr<Node<CharType>>(second.start->child[it.first]);
				}
				second.start->child.clear();
				//first.start->nfa_next_state.push_back(second.start);
				first.end->nfa_next_state.push_back(second.end);
				nfa.start = first.start;
				nfa.end = second.end;
				nfas.push(nfa);
			}
		}
		else if (ops.top() == '&')
		{
			ops.pop();
			if (nfas.size() >= 2)
			{
				//Nfa<CharType> second = nfas.top();
				//nfas.pop();
				//Nfa<CharType> first = nfas.top();
				nfas.pop();
				//Nfa<CharType> nfa;
				//nfa.start = first.start;
				//nfa.end = second.end;
				//nfas.push(nfa);
			}
		}
		else
		{
			ops.pop();
		}
	}

	template<typename CharType>
	bool MXRender::SentenceFilter<CharType>::search_sentence_recursion( std::vector < SentenceResult<CharType>>& ret, Node<CharType>* node, std::unordered_set<Node<CharType>*>& visited_set, const std::basic_string<CharType>& str,unsigned int& start, unsigned int& end, int cur,int depth)
	{
		//if (visited_set.contains(node))
		//{
		//	return false;
		//}
		//visited_set.insert(node);
		if (cur > str.size()|| depth >str.size())
		{
			return false;
		}
		if (node->key.empty() == false)
		{
			end = cur;
			ret.emplace_back(node->key, start, end);
			start = end+1;
			return true;
		}
		for (Node<CharType>* next_nfa_node : node->nfa_next_state)
		{
			if (search_sentence_recursion(ret, next_nfa_node, visited_set, str, start, end,cur, depth+1))
			{
				return true;
			}
			auto it = visited_set.find(next_nfa_node);
			if(it!=visited_set.end())
				visited_set.erase(it);
		}
		if (cur < str.size())
		{

			if (node->child.find(str[cur])!=node->child.end())
			{
				return search_sentence_recursion(ret,node->child[str[cur]].get(),visited_set,str,start,end,cur+1, depth);
			}
			else
			{
				auto it = visited_set.find(node);
				if (it != visited_set.end())
					visited_set.erase(it);
				return false;
			}
		}
		else
		{
			auto it = visited_set.find(node);
			if (it != visited_set.end())
				visited_set.erase(it);
			return false;
		}
	}


	template<typename CharType>
	bool MXRender::SentenceFilter<CharType>::check_char(char ch)
	{
		auto it= std::find(special_char.begin(), special_char.end(), ch);
		if (it == special_char.end())
		{
			return false;
		}
		return true;
	}


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
		std::unordered_set<Node<CharType>*> visited_set;
		for (int i = 0; i < str.size(); ++i)
		{
			if ((str[i] <= 0x20))
			{
				fin = i;
				continue;
			}
			if (!current->key.empty())
			{
				result.emplace_back(current->key, start, fin);
				current = root;
				start = fin;
			}
			visited_set.clear();
			for (Node<CharType>* next_nfa_node : current->nfa_next_state)
			{
				if (search_sentence_recursion(result, next_nfa_node, visited_set, str, start, fin, i,0))
				{
					current = root;
					start = i + 1;
					fin = i;
				}
			}
			if (current->child.find(str[i]) == current->child.end())
			{
				Node<CharType>* temp = current;
				current = root;
				if (temp != root)
					i--;
				start = i + 1;
				fin = i;
			}
			else
			{
				current = current->child[str[i]].get();
				fin = i+1;
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

		Node<CharType>* root = this->root.get();
		if (root == nullptr || str.empty())
		{
			return;
		}

		Node<CharType>* current = this->root.get();
		std::stack<char> ops ;
		std::stack<Nfa<CharType>> nfas ;
		bool is_pre_nfa = false;
		for(int i=0;i<str.size();i++)
		{
			if (str[i] == '(')
			{
				ops.push(str[i]);
			}
			else if (str[i] == ')')
			{
				while (ops.size() > 0)
				{
					if (ops.top() != '(')
					{
						generate_nfa(nfas, ops);
					}
					else
					{
						ops.pop();
						break;
					}
				}
			}
			else
			{
				if (check_char(str[i]) == false)
				{
					//if (current->child.find(str[i]) == current->child.end())
					//{
					//	current->child[str[i]] = std::make_unique<Node<CharType>>();
					//}
					Nfa<CharType> nfa;
					Node<CharType>* next;
					if (is_pre_nfa)
					{
						Nfa<CharType> first = nfas.top();
						nfas.pop();
						first.start->nfa_next_state.push_back(current);
						nfa.start = first.start;
					}
					else
					{
						nfa.start = current;
					}
					if (is_pre_nfa)
					{
						if (current->child.find(str[i]) == current->child.end())
						{
							current->child[str[i]] = std::make_shared<Node<CharType>>();
						}
						next= current->child[str[i]].get();
						is_pre_nfa = false;
					}
					else
					{
						if (nfa.start->child.find(str[i]) == nfa.start->child.end())
						{
							nfa.start->child[str[i]] = std::make_shared<Node<CharType>>();
						}
						next = nfa.start->child[str[i]].get();
					}

					nfa.end = next;
					nfas.push(nfa);
					if (i > 0 && str[i-1]!='|' && str[i - 1] != '(')
					{
						ops.push('&');
					}
					current = next;
					//fabc  da|bc da*bc
				}
				if (check_char(str[i]))
				{
					
					if (str[i] == '*')
					{
						Nfa<CharType> first = nfas.top();
						first.end->nfa_next_state.push_back(first.start);
						is_pre_nfa = true;
					}
					else if(str[i] == '|')
					{
						ops.push(str[i]);
					}
				}
			}
		}
		//std::unique_ptr<Node<CharType>> end = std::make_unique<Node<CharType>>();
		//current->nfa_next_state.push_back(end.get());
		//current = end.get();
		//ends.push_back(std::move(end));
		while (ops.size() > 0 && ops.top() != '(')
		{
			generate_nfa(nfas, ops);
		}

		current->key = str;
		
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
		void insert(const std::string& keyword );
		std::vector<SentenceResult<char>> search(const std::string& str);
		std::string replace(const std::string& str, char replace_char);
		bool find(const std::string& str);
	protected:
		SentenceFilter<char> sentence_filter;

	};


}
#endif
