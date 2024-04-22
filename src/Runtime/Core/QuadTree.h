#pragma once
#ifndef _QUADTREE_
#define _QUADTREE_
#include <memory>
#include "ConstDefine.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)




MYRENDERER_BEGIN_CLASS(QuadTreeNode)

#pragma region MATHOD

public:
	QuadTreeNode* left_top = nullptr;
	QuadTreeNode* right_top = nullptr;
	QuadTreeNode* left_bottom = nullptr;
	QuadTreeNode* right_bottom = nullptr;
	UInt32 level = 0;
	UInt32 node_id = 0;
	UInt32 x = 0;
	UInt32 y = 0;
	UInt32 size = 0;
	VIRTUAL ~QuadTreeNode() MYDEFAULT;
protected:

private:

#pragma endregion

#pragma region MEMBER

public:

protected:

private:

#pragma endregion

MYRENDERER_END_CLASS


MYRENDERER_TEMPLATE_HEAD(typename NodeType)
MYRENDERER_BEGIN_CLASS(QuadTree)

#pragma region MATHOD

public:
	QuadTree(UInt32 in_max_size, UInt32 in_single_block_size)
		: max_size(in_max_size)
		, single_block_size(in_single_block_size)
	{
		int node_size = in_max_size;
		num_level = 0;
		while (node_size >= in_single_block_size)
		{
			num_level++;
			node_size = (node_size >> 1);
		}
		if (num_level > 0)
		{
			root_width = 1;
			root_height = 1;
		}
		InitNodeLevel();
	}
	VIRTUAL ~QuadTree() MYDEFAULT;

	VIRTUAL void InitNodeLevel()
	{
		node_level.resize(num_level);
		for (int i = 0; i < num_level; i++)
		{
			int width = (root_width << i);
			int height = (root_height << i);
			node_level[i].resize(width * height);
			int node_size = max_size >> (i * 2);
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					int node_id = x + y * width;
					NodeType& node = node_level[i][node_id];
					node.node_id = node_id;
					node.level = i;
					node.x = x;
					node.y = y;
					node.size = node_size;
				}
			}
		}
		for (int i = 0; i < num_level - 1; i++)
		{
			int width = (root_width << i);
			int height = (root_height << i);
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					int node_id = x + y * width;
					NodeType& node = node_level[i][node_id];
					if (node_level.size() > (i + 1))
					{
						int max_sub_id = max(x * 2 + (y * 2) * (width * 2), max(x * 2 + (y * 2) * (width * 2) + 1, max(x * 2 + (y * 2 + 1) * (width * 2), x * 2 + (y * 2 + 1) * (width * 2) + 1)));
						if (max_sub_id >= node_level[i + 1].size())
						{
							continue;
						}
						node.left_top = &node_level[i + 1][x * 2 + (y * 2) * (width * 2)];
						node.right_top = &node_level[i + 1][x * 2 + (y * 2) * (width * 2) + 1];
						node.left_bottom = &node_level[i + 1][x * 2 + (y * 2 + 1) * (width * 2)];
						node.right_bottom = &node_level[i + 1][x * 2 + (y * 2 + 1) * (width * 2) + 1];
					}
				}
			}
		}
	}



	void ParseAllLevelNodeToDebug()
	{
		for (int i = 0; i < num_level - 1; i++)
		{
			std::cout << std::endl;
			std::cout << std::endl;
			std::cout << "Level:" << i << std::endl;
			int width = (root_width << i);
			int height = (root_height << i);
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					int node_id = x + y * width;
					NodeType& node = node_level[i][node_id];
					std::cout << "Node:" << node_id << "  ";
				}
				std::cout << std::endl;
			}
		}

	}

	

protected:

	Vector<Vector<NodeType>> node_level;
	UInt32 max_size = 0;
	UInt32 single_block_size = 0;
	UInt32 num_level = 0,root_width=0,root_height=0;
private:

#pragma endregion

#pragma region MEMBER

public:

protected:

private:

#pragma endregion

MYRENDERER_END_CLASS




MYRENDERER_END_NAMESPACE

#endif

