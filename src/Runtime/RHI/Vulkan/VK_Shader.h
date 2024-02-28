#pragma once

#ifndef _VK_SHADER_
#define _VK_SHADER_
#include "RHI/RenderShader.h"
#include "VK_Device.h"
#include "VK_Define.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_Device;

MYRENDERER_BEGIN_STRUCT(DescriptorSetLayoutData)
UInt8 set_number;
VkDescriptorSetLayoutCreateInfo create_info;
Vector<VkDescriptorSetLayoutBinding> bindings;
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_STRUCT(ReflectedBinding)
UInt8 set;
UInt8 binding;
VkDescriptorType type;
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_STRUCT(ReflectedConstantInfo)
String name;
VkPushConstantRange constant;
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_STRUCT(ReflectedInfo)
Map<String, ReflectedBinding> bindings;
Vector<DescriptorSetLayoutData> setlayouts;
Vector<ReflectedConstantInfo> constant_ranges;
ReflectedInfo() DEFAULT;
ReflectedInfo( CONST Vector<DescriptorSetLayoutData>& in_setlayouts, CONST Vector<ReflectedConstantInfo>& in_constant_ranges) :
	 setlayouts(in_setlayouts), constant_ranges(in_constant_ranges) {}
ReflectedInfo(CONST ReflectedInfo& in_reflect) :
	 setlayouts(in_reflect.setlayouts), constant_ranges(in_reflect.constant_ranges) {}
MYRENDERER_END_STRUCT


MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_ShaderResourceBinding, public ShaderResourceBinding)
friend class VK_PipelineState;
#pragma region METHOD
public:
	VK_ShaderResourceBinding(Map<String, ReflectedBinding>& in_bindings) ;

	VIRTUAL ~VK_ShaderResourceBinding();
	CONST VkDescriptorSet* METHOD(GetDescriptorSets)() CONST;
protected:

private:

#pragma endregion

#pragma region MEMBER
public:
protected:
	Array<VkDescriptorSet, MYRENDER_MAX_BINDING_SET_NUM> descriptorset{ VK_NULL_HANDLE ,VK_NULL_HANDLE ,VK_NULL_HANDLE ,VK_NULL_HANDLE };
	Map<String, ReflectedBinding>& bindings;
private:

#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_Shader, public Shader)


#pragma region METHOD
public:
	VK_Shader(VK_Device* in_device, CONST ShaderDesc& desc, CONST ShaderDataPayload& data);

	void METHOD(Destroy)();
	VIRTUAL ~VK_Shader();
	VkPipelineLayout METHOD(GetBuiltLayout)();
	VkShaderModule METHOD(GetShaderModule)() CONST;
	CONST ReflectedInfo& METHOD(GetReflectedInfo)()CONST;
protected:
	Vector<UInt32> METHOD(ReadFile)(CONST String& filename);
	VkShaderModule METHOD(CreateShaderModule)(CONST Vector<UInt32>& code);
	void METHOD(ReflectBindings)();
private:

#pragma endregion


#pragma region MEMBER
public:

protected:

	VK_Device* device;

	ReflectedInfo reflect_info;

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
