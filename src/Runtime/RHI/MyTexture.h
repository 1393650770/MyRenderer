#pragma once

#ifndef _My_TEXTURE_
#define _My_TEXTURE_

#include "../Core/ConstDefine.h"
#include "RenderEnum.h"
#include "RenderRource.h"

#include"GLFW/glfw3.h"
#include <vector>
#include <string>


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

MYRENDERER_BEGIN_STRUCT(MipmapInfo)
	size_t dataSize;
	size_t dataOffset;
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_STRUCT(TextureDesc)
	TextureDesc() DEFAULT;
	TextureDesc(CONST TextureDesc& other);

	TextureDesc& operator=(CONST TextureDesc& other)
	{
		width			= other.width;
		height				= other.height;
		mip_level				= other.mip_level;
		layer_count			= other.layer_count;
		format				= other.format;
		type				= other.type;
		depth			= other.depth;
		samples				= other.samples;
		clear_value			= other.clear_value;
		usage            = other.usage;
		return *this;
	}

	UInt32 width=0;
	UInt32 height=0;
	UInt8 mip_level=0;
	UInt8 layer_count=1;
	ENUM_TEXTURE_FORMAT format=ENUM_TEXTURE_FORMAT::None;
	ENUM_TEXTURE_TYPE type=ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
	ENUM_TEXTURE_USAGE_TYPE usage=ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_NOT_VALID;
	UInt16 depth=1;
	UInt8 samples=1;

	union ClearValue
	{
		Float32 color[4];
		Float32 ds_value[2];
	} clear_value;

MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_STRUCT(TextureDataPayload)
	TextureDataPayload() DEFAULT;
	TextureDataPayload(CONST TextureDataPayload& other);

	TextureDataPayload& operator=(CONST TextureDataPayload& other)
	{
		data			= other.data;
		data_size				= other.data_size;
		mip_level				= other.mip_level;
		layer_count			= other.layer_count;
		format				= other.format;
		type				= other.type;
		return *this;
	}

	void* data = nullptr;
	size_t data_size = 0;
	UInt8 mip_level = 1;
	UInt8 layer_count = 1;
	ENUM_TEXTURE_FORMAT format=ENUM_TEXTURE_FORMAT::None;
	ENUM_TEXTURE_TYPE type=ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;

MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(Texture,public RenderResource)
#pragma region METHOD
public:
	Texture(CONST TextureDesc in_texture_desc );
	VIRTUAL ~Texture() DEFAULT;

	VIRTUAL TextureDesc METHOD(GetTextureDesc)() CONST ; 

	VIRTUAL void METHOD(UpdateTextureData)(CONST TextureDataPayload& texture_data_payload);
protected:
	
private:

#pragma endregion


#pragma region MEMBER
public:

protected:
	TextureDesc texture_desc;
private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif