#pragma once

#ifndef _VK_SHADER_
#define _VK_SHADER_
#include "../Shader.h"
#include "VK_Device.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_Device;

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_Shader, public Shader)

struct ReflectionOverrides;

#pragma region METHOD
public:
	VK_Shader(VK_Device* in_device,Vector<ShaderData>& in_shader_data);

	VkPipelineLayout METHOD(get_built_layout)();

	void METHOD(fill_stages)(Vector<VkPipelineShaderStageCreateInfo>& pipeline_stages);
	void METHOD(reflect_layout)(ReflectionOverrides* overrides, int override_count);
	void METHOD(build_sets)(VkDevice device, VkDescriptorPool descript_pool);
	void METHOD(destroy)();
	VIRTUAL ~VK_Shader();

protected:

private:
	Vector<UInt32> METHOD(read_file)(CONST String& filename);
	VkShaderModule METHOD(create_shader_module)(CONST Vector<UInt32>& code);
	std::tuple<VkDeviceSize, VkBuffer, VkDeviceMemory>& getUniformTuple(CONST String& name);
#pragma endregion


#pragma region MEMBER
public:
	struct ReflectionOverrides {
		CONST Char* name;
		VkDescriptorType override_type;
	};
	struct DescriptorSetLayoutData {
		UInt32 set_number;
		VkDescriptorSetLayoutCreateInfo create_info;
		Vector<VkDescriptorSetLayoutBinding> bindings;
	};
	struct ReflectedBinding {
		UInt32 set;
		UInt32 binding;
		VkDescriptorType type;
	};
protected:
	// ≥Ã–ÚID
	UInt32 id;
	VK_Device* device;
	mutable Map<String, std::tuple<VkDeviceSize, VkBuffer, VkDeviceMemory>> uniform_map;
	Map<String, ReflectedBinding> bindings;
	Array<VkDescriptorSetLayout, 4> set_layouts{ VK_NULL_HANDLE ,VK_NULL_HANDLE ,VK_NULL_HANDLE ,VK_NULL_HANDLE };
	Array<UInt32, 4> set_hashes;
	Array<VkDescriptorSet, 4> sets;
	VkPipelineLayout built_layout = VK_NULL_HANDLE;
	VkShaderModule shader_modules[ENUM_SHADER_STAGE::NumStages]{ VK_NULL_HANDLE };
	Vector<UInt32> shader_codes[ENUM_SHADER_STAGE::NumStages];

private:
#pragma endregion

    public:

    private:
    protected:
       
    public:

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif //_VK_SHADER_
