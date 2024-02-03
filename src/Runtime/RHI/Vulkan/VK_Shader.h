#pragma once

#ifndef _VK_SHADER_
#define _VK_SHADER_
#include "RHI/Shader.h"
#include "VK_Device.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_Device;

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_Shader, public Shader)

struct ReflectionOverrides;

#pragma region METHOD
public:
	VK_Shader(VK_Device* in_device, CONST ShaderDesc& desc, CONST ShaderDataPayload& data);

	VkPipelineLayout METHOD(GetBuiltLayout)();

	void METHOD(Destroy)();
	VIRTUAL ~VK_Shader();

protected:

private:
	Vector<UInt32> METHOD(ReadFile)(CONST String& filename);
	VkShaderModule METHOD(CreateShaderModule)(CONST Vector<UInt32>& code);
	std::tuple<VkDeviceSize, VkBuffer, VkDeviceMemory>& GetUniformTuple(CONST String& name);
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
	VkShaderModule shader_modules= VK_NULL_HANDLE ;
	Vector<UInt32> shader_codes;

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
