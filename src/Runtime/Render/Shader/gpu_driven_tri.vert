#version 460
#extension GL_GOOGLE_include_directive : require

#include "common.glsl"
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vNormal;
layout (location = 2) in vec3 vColor;
layout (location = 3) in vec2 vTexCoord;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 texCoord;
layout (location = 2) out vec3 outNormal;
layout(set=0,binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;



struct ObjectData{
mat4 model;
vec4 spherebounds;
vec4 extents;
}; 


//all object matrices
layout(std140,set = 1, binding = 0) readonly buffer ObjectBuffer{   

	ObjectData objects[];
} objectBuffer;

//all object indices
layout(set = 1, binding = 1) readonly buffer InstanceBuffer{   

	uint IDs[];
} instanceBuffer;

void main() 
{	
	uint index = instanceBuffer.IDs[gl_InstanceIndex];
	
	vec3 vec_tre_Normal = OctNormalDecode(vNormal);

	mat4 modelMatrix = objectBuffer.objects[index].model;
	mat4 transformMatrix = (mvp.proj* mvp.view * modelMatrix);
	gl_Position = transformMatrix * vec4(vPosition, 1.0f);
	outNormal = normalize((modelMatrix * vec4(vec_tre_Normal,0.f)).xyz);
	outColor = vColor;
	texCoord = vTexCoord;

}
