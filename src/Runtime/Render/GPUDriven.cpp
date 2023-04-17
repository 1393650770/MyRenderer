#include"GPUDriven.h"
#include <vector>



namespace MXRender
{

	GPUDrivenSystem::GPUDrivenSystem()
	{

	}

	GPUDrivenSystem::~GPUDrivenSystem()
	{

	}

	void GPUDrivenSystem::excute_upload_computepass()
	{
		//std::vector<VkBufferCopy> copies;
		//int object_size= 1;
		//copies.reserve(object_size);

		//VK_GraphicsContext* context= Singleton<DefaultSetting>::get_instance().context.get();
		//PipelineShaderObject* upload_comp= context->material_system.psos["upload_comp"];
		//uint64_t buffersize = sizeof(GPUObjectData) * object_size;
		//uint64_t vec4size = sizeof(glm::vec4);
		//uint64_t intsize = sizeof(uint32_t);
		//uint64_t wordsize = sizeof(GPUObjectData) / sizeof(uint32_t);
		//uint64_t uploadSize = object_size * wordsize * intsize;
		//AllocatedBufferUntyped gpudata_newBuffer = VK_Utils::Create_buffer(context, buffersize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		//AllocatedBufferUntyped id_targetBuffer = VK_Utils::Create_buffer(context,uploadSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


		//uint32_t* targetData ;
		//vmaMapMemory(context->_allocator, id_targetBuffer._allocation, (void**)&targetData);
		//GPUObjectData* objectSSBO ;
		//vmaMapMemory(context->_allocator, gpudata_newBuffer._allocation, (void**)&objectSSBO);
		//uint32_t launchcount = static_cast<uint32_t>(object_size * wordsize);
		//{

		//	uint32_t sidx = 0;
		//	for (int i = 0; i < object_size; i++)
		//	{
		//		//_renderScene.write_object(objectSSBO + i, object_size[i]);


		//		uint32_t dstOffset = static_cast<uint32_t>(wordsize * object_size[i].handle);

		//		for (int b = 0; b < wordsize; b++)
		//		{
		//			uint32_t tidx = dstOffset + b;
		//			targetData[sidx] = tidx;
		//			sidx++;
		//		}
		//	}
		//	launchcount = sidx;
		//}
		//vmaUnmapMemory(context->_allocator,gpudata_newBuffer._allocation);
		//vmaUnmapMemory(context->_allocator, id_targetBuffer._allocation);

		//VkDescriptorBufferInfo indexData = id_targetBuffer.get_info();

		//VkDescriptorBufferInfo sourceData = gpudata_newBuffer.get_info();

		//VkDescriptorBufferInfo targetInfo ;//= _renderScene.objectDataBuffer.get_info();

		//VkDescriptorSet COMPObjectDataSet;
		//DescriptorBuilder::begin(context->material_system.get_descript_pool());
		//	.bind_buffer(0, &indexData, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		//	.bind_buffer(1, &sourceData, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		//	.bind_buffer(2, &targetInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		//	.build(COMPObjectDataSet);

		//VK_Utils::Immediate_Submit(context, 
		//	[&](VkCommandBuffer cmd)
		//	{
		//		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, upload_comp->pipeline);


		//		vkCmdPushConstants(cmd, upload_comp->pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &launchcount);

		//		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, upload_comp->pipeline_layout, 0, 1, &COMPObjectDataSet, 0, nullptr);

		//		vkCmdDispatch(cmd, ((launchcount) / 256) + 1, 1, 1);
		//	}
		//);
	}


	//{

	//	std::vector<Vertex> vertices1 = { ... }; // 模型1的顶点数据
	//	std::vector<uint32_t> indices1 = { ... }; // 模型1的索引数据

	//	std::vector<Vertex> vertices2 = { ... }; // 模型2的顶点数据
	//	std::vector<uint32_t> indices2 = { ... }; // 模型2的索引数据

	//	// 将模型数据存储到缓冲对象中
	//	VkBuffer vertexBuffer;
	//	VkBuffer indexBuffer;
	//	VkDeviceSize vertexOffset1 = 0;
	//	VkDeviceSize vertexOffset2 = vertices1.size() * sizeof(Vertex);
	//	VkDeviceSize indexOffset1 = 0;
	//	VkDeviceSize indexOffset2 = indices1.size() * sizeof(uint32_t);

	//	createBuffer(device, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);
	//	createBuffer(device, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBuffer, indexBufferMemory);

	//	void* data;
	//	vkMapMemory(device, vertexBufferMemory, 0, vertexBufferSize, 0, &data);
	//	memcpy(data, vertices1.data(), vertices1.size() * sizeof(Vertex));
	//	memcpy(data + vertexOffset2, vertices2.data(), vertices2.size() * sizeof(Vertex));
	//	vkUnmapMemory(device, vertexBufferMemory);

	//	vkMapMemory(device, indexBufferMemory, 0, indexBufferSize, 0, &data);
	//	memcpy(data, indices1.data(), indices1.size() * sizeof(uint32_t));
	//	memcpy(data + indexOffset2, indices2.data(), indices2.size() * sizeof(uint32_t));
	//	vkUnmapMemory(device, indexBufferMemory);

	//	// 创建绘制命令缓冲
	//	VkCommandBuffer commandBuffer = ...; // 创建一个 VkCommandBuffer 对象
	//	VkDeviceSize offsets[1] = { 0 };

	//	// 定义两个绘制命令
	//	VkDrawIndexedIndirectCommand drawCmd1 = {};
	//	drawCmd1.indexCount = indices1.size();
	//	drawCmd1.instanceCount = 10; // 绘制10个模型1的实例
	//	drawCmd1.firstIndex = 0;
	//	drawCmd1.vertexOffset = vertexOffset1;
	//	drawCmd1.firstInstance = 0;

	//	VkDrawIndexedIndirectCommand drawCmd2 = {};
	//	drawCmd2.indexCount = indices2.size();
	//	drawCmd2.instanceCount = 5; // 绘制5个模型2的实例
	//	drawCmd2.firstIndex = 0;
	//	drawCmd2.vertexOffset = vertexOffset2;
	//	drawCmd2.firstInstance = 10; // 模型1的实例数量是10

	//	// 创建一个缓冲对象，用于存储绘制命令
	//	VkBuffer drawBuffer;
	//	VkDeviceSize drawBufferSize = 2 * sizeof(VkDrawIndexedIndirectCommand);
	//	createBuffer(device, drawBufferSize, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, drawBuffer, drawBufferMemory);

	//	// 将绘制命令存储到缓冲对象中
	//	void* drawData;
	//	vkMapMemory(device, drawBufferMemory, 0, drawBufferSize, 0, &drawData);
	//	memcpy(drawData, &drawCmd1, sizeof(VkDrawIndexedIndirectCommand));
	//	memcpy(drawData + sizeof(VkDrawIndexedIndirectCommand), &drawCmd2, sizeof(VkDrawIndexedIndirectCommand));
	//	vkUnmapMemory(device, drawBufferMemory);

	//	// 执行绘制命令
	//	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
	//	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	//	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
	//	vkCmdDrawIndexedIndirect(commandBuffer, drawBuffer, 0, 2, sizeof(VkDrawIndexedIndirectCommand));


	//}

}