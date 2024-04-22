#pragma once
#ifndef _VIRTUALTEXTUREDATAQUADTREE_
#define _VIRTUALTEXTUREDATAQUADTREE_
#include <memory>
#include "Core/QuadTree.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)
MYRENDERER_BEGIN_NAMESPACE(Core)


MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VirtualTextureQuadTreeNode,public QuadTreeNode)

#pragma region MATHOD

public:

	VIRTUAL ~VirtualTextureQuadTreeNode() MYDEFAULT;
protected:

private:

#pragma endregion

#pragma region MEMBER

public:

protected:

private:

#pragma endregion

MYRENDERER_END_CLASS



MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VirtualTextureQuadTree,public QuadTree<VirtualTextureQuadTreeNode>)

#pragma region MATHOD

public:
	VirtualTextureQuadTree(UInt32 in_max_size, UInt32 in_single_block_size);
	VIRTUAL ~VirtualTextureQuadTree() MYDEFAULT;
	void ParseAllLevelFromRoot()
	{
		int current_level = 0;
		Queue<QuadTreeNode*> node_que;
		node_que.push(&node_level[current_level][0]);
		int current_que_size = 1;
		while (node_que.empty() == false)
		{
			std::cout << std::endl;
			std::cout << std::endl;
			std::cout << "Level:" << current_level << std::endl;
			int width = (root_width << current_level);
			int height = (root_height << current_level);
			int cur_x = 0;

			while (current_que_size > 0)
			{
				QuadTreeNode* node = node_que.front();
				std::cout << "Node:" << node->node_id << "  ";
				node_que.pop();
				current_que_size--;
				cur_x++;
				if (cur_x >= width)
				{
					std::cout << std::endl;
					cur_x = 0;
				}
				if (node->left_top != nullptr)
				{
					node_que.push(node->left_top);
				}
				if (node->right_top != nullptr)
				{
					node_que.push(node->right_top);
				}
				if (node->left_bottom != nullptr)
				{
					node_que.push(node->left_bottom);
				}
				if (node->right_bottom != nullptr)
				{
					node_que.push(node->right_bottom);
				}

			}
			std::cout << std::endl;
			if (current_que_size == 0)
			{
				current_level++;
				current_que_size = node_que.size();
			}
		}

	}
protected:

private:

#pragma endregion

#pragma region MEMBER

public:

protected:

private:

#pragma endregion

MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif

