#version 450
#extension GL_ARB_separate_shader_objects : enable

//when we start using vertex buffers
//layout(location = 0) in vec3 inPosition;
//layout(location = 1) in vec3 inNormal;
//layout(location = 2) in vec2 inTexCoord;


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

//to fragment shader
layout(location = 0) out vec3 position;
layout(location = 1) out vec3 color;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.85f),
    vec2(0.8f, 0.7f),
    vec2(-0.8f, 0.7f)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    color = colors[gl_VertexIndex];
}