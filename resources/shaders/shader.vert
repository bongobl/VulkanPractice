#version 450
#extension GL_ARB_separate_shader_objects : enable

//In parameters (This vertex's attributes)
layout (location = 0) in vec3 inPosition;
layout (location = 1) in float mass;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 inTexCoord;
layout (location = 4) in vec3 velocity;


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;

} ubo;

//Out parameters
layout (location = 0) out vec3 modelSpacePosition;
layout (location = 1) out vec3 modelSpaceNormal;
layout (location = 2) out vec2 texCoord;

void main() {

    modelSpacePosition = inPosition;
    modelSpaceNormal = inNormal;
    texCoord = inTexCoord;

    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0f);

    gl_PointSize = 2 * mass / gl_Position.z;

}