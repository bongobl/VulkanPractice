#version 450
#extension GL_ARB_separate_shader_objects : enable

//from vertex shader
layout(location = 0) in vec3 modelSpacePosition;
layout(location = 1) in vec3 modelSpaceNormal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in mat4 modelMatrix;
layout(location = 7) in vec3 lightDirection;
layout(location = 8) in vec3 cameraPosition;
layout(location = 9) in vec3 matColor;

//output to color attachment
layout(location = 0) out vec4 outColor;

void main() {
	
	vec3 worldPosition = vec3(modelMatrix * vec4(modelSpacePosition,1));

	mat3 toWorldMat3 = transpose(inverse(mat3(modelMatrix)));

	vec3 worldNormal = normalize(toWorldMat3 * modelSpaceNormal);


	vec3 diffuse = matColor * max(0, dot(-lightDirection, worldNormal));

	vec3 reflectedLight = reflect(lightDirection, worldNormal);
	vec3 fragToCam = normalize(cameraPosition - worldPosition);
	vec3 specular = pow(max(0, dot(reflectedLight, fragToCam)),20) * vec3(1.0f);

	vec3 ambient = 0.04f * matColor;

	vec3 finalColor = diffuse + specular + ambient; 


    outColor = vec4(finalColor, 1.0);
}