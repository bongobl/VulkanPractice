#version 450
#extension GL_ARB_separate_shader_objects : enable

//This vertex's attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;


//to fragment shader
layout(location = 0) out vec3 worldPosition;
layout(location = 1) out vec3 worldNormal;
layout(location = 2) out vec2 texCoord;



void main() {
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0);

    worldPosition = vec3(ubo.model * vec4(inPosition, 1.0));
    worldNormal = inNormal;//vec3(ubo.projection * ubo.view * ubo.model * vec4(inNormal, 0.0));
    texCoord = inTexCoord;
}