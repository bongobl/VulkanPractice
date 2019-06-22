#version 450
#extension GL_ARB_separate_shader_objects : enable

//from vertex shader
layout(location = 0) in vec3 modelSpacePosition;
layout(location = 1) in vec3 modelSpaceNormal;
layout(location = 2) in vec2 texCoord;


//Descriptor Bindings
layout(binding = 4) uniform UniformBufferObject{

	vec3 lightDirection;
    float textureParam;
    vec3 cameraPosition;
    float normalMapStrength;
    vec3 matColor;
    float padding3;
    mat4 lightVP;

} ubo;



//output to color attachment
layout(location = 0) out vec4 outColor;


void main() {
	

	outColor.rgb = modelSpaceNormal;
	
    vec2 coord = gl_PointCoord - vec2(0.5);  //from [0,1] to [-0.5,0.5]
	if(length(coord) > 0.5)                  //outside of circle radius?
	    discard;



}