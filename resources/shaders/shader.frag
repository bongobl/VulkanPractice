#version 450
#extension GL_ARB_separate_shader_objects : enable

//from vertex shader
layout(location = 0) in vec3 worldPosition;
layout(location = 1) in vec3 worldNormal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(worldNormal, 1.0);
}