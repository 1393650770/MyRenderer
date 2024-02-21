#include "Test.h"
#include "Render/Core/RenderGraph.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

void RenderPipelineTest::Run()
{
	RenderGraph graph;
	struct TestData
	{
		MXRender::RHI::Texture* output;
	};
	graph.AddRenderPass<TestData>("TestPass",
	[&](TestData& data, RenderGraphPassBuilder& builder)
	{

	},
	[=](const TestData& data)
	{

	});

	graph.Compile();
	graph.Execute();
	graph.Release();
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
