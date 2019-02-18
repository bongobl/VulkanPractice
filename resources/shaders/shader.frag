#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D diffuseTexture;
layout(binding = 2) uniform samplerCube envMap;

//from vertex shader
layout(location = 0) in vec3 modelSpacePosition;
layout(location = 1) in vec3 modelSpaceNormal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in mat4 modelMatrix;
layout(location = 7) in vec3 lightDirection;
layout(location = 8) in float textureParam;
layout(location = 9) in vec3 cameraPosition;
layout(location = 10) in vec3 matColor;


//output to color attachment
layout(location = 0) out vec4 outColor;

void main() {
	
	//world space correction
	vec3 worldPosition = vec3(modelMatrix * vec4(modelSpacePosition,1));
	mat3 toWorldMat3 = transpose(inverse(mat3(modelMatrix)));
	vec3 worldNormal = normalize(toWorldMat3 * modelSpaceNormal);
	

	//environment map reflection
	vec3 reflectedCam = reflect(worldPosition - cameraPosition, worldNormal);
	vec3 reflectiveColor = texture(envMap,reflectedCam).rgb;

	//diffuse texture
	vec3 diffuseTextureColor = vec3(texture(diffuseTexture, texCoord));

	//combine env map and diffuse texture
	vec3 combined = (1 - textureParam) * diffuseTextureColor + textureParam * reflectiveColor;

	//diffuse
	vec3 diffuse = matColor * combined * max(0, dot(-lightDirection, worldNormal));

	//specular
	vec3 reflectedLight = reflect(lightDirection, worldNormal);
	vec3 fragToCam = normalize(cameraPosition - worldPosition);
	vec3 specular = pow(max(0, dot(reflectedLight, fragToCam)),10) * vec3(1.0f);

	//ambient
	vec3 ambient = 0.04f * matColor;


	//final color
	outColor.rgb = diffuse + specular + ambient; 
    outColor.a = 1.0f;
}