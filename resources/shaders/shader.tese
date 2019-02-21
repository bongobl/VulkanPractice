#version 450
#extension GL_ARB_separate_shader_objects : enable

//Layout specification.
layout (triangles, equal_spacing, ccw) in;
 
//In parameters.
layout (location = 0) in vec3 positions[];
layout (location = 1) in vec3 normals[];
layout (location = 2) in vec2 texCoords[];

 
//Texture samplers (might have to change binding)
layout (binding = 3) uniform sampler2D displacementTexture;
 
//Out parameters.
layout (location = 0) out vec3 fragPosition;
layout (location = 1) out vec3 fragNormal;
layout (location = 2) out vec2 fragTexCoord;

void main()
{
    // Interpolate vertex attributes and pass along to the fragment shader.
    fragTexCoord = gl_TessCoord.x * texCoords[0] + gl_TessCoord.y * texCoords[1] + gl_TessCoord.z * texCoords[2];
    fragNormal = gl_TessCoord.x * normals[0] + gl_TessCoord.y * normals[1] + gl_TessCoord.z * normals[2];
    fragPosition = gl_TessCoord.x * positions[0] + gl_TessCoord.y * positions[1] + gl_TessCoord.z * positions[2];
 
    // uncomment when ready to use
    //float displacement = texture(displacementTexture, fragTexCoord).r;
 	//fragPosition.y += displacement;

    
 
    gl_Position = vec4(fragPosition, 1.0f);
}