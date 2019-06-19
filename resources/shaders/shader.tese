#version 450
#extension GL_ARB_separate_shader_objects : enable

//Layout specification.
layout (triangles, equal_spacing, cw) in;
 
//In parameters
layout (location = 0) in vec3 contPointPositions[];
layout (location = 1) in vec3 contPointNormals[];
layout (location = 2) in vec2 contPointTexCoords[];


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;

} ubo;
 
//Out parameters
layout (location = 0) out vec3 innerPosition;
layout (location = 1) out vec3 innerNormal;
layout (location = 2) out vec2 innerTexCoord;
layout (location = 3) out mat4 modelMatrix;

void main()
{
    // Interpolate vertex attributes and pass along to the fragment shader.   
	innerPosition = gl_TessCoord.x * contPointPositions[0] + gl_TessCoord.y * contPointPositions[1] + gl_TessCoord.z * contPointPositions[2];
    innerNormal = gl_TessCoord.x * contPointNormals[0] + gl_TessCoord.y * contPointNormals[1] + gl_TessCoord.z * contPointNormals[2];
    innerTexCoord = gl_TessCoord.x * contPointTexCoords[0] + gl_TessCoord.y * contPointTexCoords[1] + gl_TessCoord.z * contPointTexCoords[2];
	modelMatrix = ubo.model;


    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(innerPosition, 1.0f);



}

