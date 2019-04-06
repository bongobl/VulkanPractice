#version 450
#extension GL_ARB_separate_shader_objects : enable

//from tesselation eval shader
layout(location = 0) in vec3 modelSpacePosition;
layout(location = 1) in vec3 modelSpaceNormal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in mat4 modelMatrix;

//output to depth image attachment
//layout(location = 0) out vec4 outDepth;

void main() {
	return;
}