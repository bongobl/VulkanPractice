#version 450
#extension GL_ARB_separate_shader_objects : enable

//Layout specification.
layout (triangles, equal_spacing, ccw) in;
 
//In parameters
layout (location = 0) in vec3 contPointPositions[];
layout (location = 1) in vec3 contPointNormals[];
layout (location = 2) in vec2 contPointTexCoords[];

 
//Texture samplers (might have to change binding)
layout (binding = 3) uniform sampler2D displacementTexture;
 
//Out parameters
layout (location = 0) out vec3 fragPosition;
layout (location = 1) out vec3 fragNormal;
layout (location = 2) out vec2 fragTexCoord;

void main()
{
    // Interpolate vertex attributes and pass along to the fragment shader.   
	fragPosition = gl_TessCoord.x * contPointPositions[0] + gl_TessCoord.y * contPointPositions[1] + gl_TessCoord.z * contPointPositions[2];
    fragNormal = gl_TessCoord.x * contPointNormals[0] + gl_TessCoord.y * contPointNormals[1] + gl_TessCoord.z * contPointNormals[2];
    fragTexCoord = gl_TessCoord.x * contPointTexCoords[0] + gl_TessCoord.y * contPointTexCoords[1] + gl_TessCoord.z * contPointTexCoords[2];
 
    // uncomment when ready to use
    //float displacement = texture(displacementTexture, fragTexCoord).r;
 	//fragPosition.y += displacement;

    
 
    gl_Position = vec4(fragPosition, 1.0f);
}