#include "VertexBuffer.h"

namespace MXRender
{
	VertexBuffer::~VertexBuffer()
	{
	}

	Layout::Layout(const std::initializer_list<Layout_Element>& elements):layout(elements)
	{
		stride = 0;
		for (auto& element : layout)
		{
			offset[element.attributr_type] = stride;
			stride += element.num;
		}
	}


	Layout::~Layout()
	{
	}
}

