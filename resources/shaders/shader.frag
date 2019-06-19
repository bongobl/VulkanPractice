#version 450
#extension GL_ARB_separate_shader_objects : enable

//from tesselation eval shader
layout(location = 0) in vec3 modelSpacePosition;
layout(location = 1) in vec3 modelSpaceNormal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in mat4 modelMatrix;


//Descriptor Bindings
layout(binding = 1) uniform sampler2D diffuseTexture;
layout(binding = 2) uniform sampler2D normalTexture;
layout(binding = 3) uniform samplerCube envMap;
layout(binding = 4) uniform UniformBufferObject{

	vec3 lightDirection;
    float textureParam;
    vec3 cameraPosition;
    float normalMapStrength;
    vec3 matColor;
    float padding3;
    mat4 lightVP;

} ubo;

layout(binding = 5) uniform sampler2D shadowMap;


//output to color attachment
layout(location = 0) out vec4 outColor;


void main() {
	
	//world space correction
	vec3 worldPosition = vec3(modelMatrix * vec4(modelSpacePosition,1));
	mat3 toWorldMat3 = transpose(inverse(mat3(modelMatrix)));
	vec3 worldNormal = normalize(toWorldMat3 * modelSpaceNormal);


	//apply normal texture offsets
	worldNormal = normalize(worldNormal + ubo.normalMapStrength * (texture(normalTexture, texCoord).xyz * 2.0f - 1.0f));

	//environment map reflection
	vec3 reflectedCam = reflect(worldPosition - ubo.cameraPosition, worldNormal);
	vec3 reflectiveColor = texture(envMap,reflectedCam).rgb;
	
	//diffuse texture
	vec3 diffuseTextureColor = texture(diffuseTexture, texCoord).rgb;
	
	//combine env map and diffuse texture
	vec3 combined = (1 - ubo.textureParam) * diffuseTextureColor + ubo.textureParam * reflectiveColor;

	//diffuse
	vec3 diffuse = ubo.matColor * combined * max(0, dot(-ubo.lightDirection, worldNormal));
	
	//specular
	vec3 reflectedLight = reflect(ubo.lightDirection, worldNormal);
	vec3 fragToCam = normalize(ubo.cameraPosition - worldPosition);
	vec3 specular = pow(max(0, dot(reflectedLight, fragToCam)),10) * vec3(1.0f);

	//ambient
	vec3 ambient = 0.015f * ubo.matColor * combined;

	//final color
	outColor.rgb = diffuse + specular + ambient; 
	
    outColor.a = 1.0f;


}