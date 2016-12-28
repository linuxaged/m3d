#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 inPos;

layout (binding = 0) uniform UBO 
{
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
} ubo;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	gl_Position = inPos * ubo.modelMatrix * ubo.viewMatrix * ubo.projectionMatrix;
	//gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * inPos;
}
