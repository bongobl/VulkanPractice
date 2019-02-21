#version 450
#extension GL_ARB_separate_shader_objects : enable

//Layout specification.
layout (vertices = 3) out;
 
//In parameters.
layout (location = 0) in vec3 vertexPositions[];
layout (location = 1) in vec3 vertexNormals[];
layout (location = 2) in vec2 vertexTexCoords[];


//Out parameters.
layout (location = 0) out vec3 positions[];
layout (location = 1) out vec3 normals[];
layout (location = 2) out vec2 texCoords[];



vec3 GetMiddlePoint(vec3 v1, vec3 v2){
    return 0.5f * v1 + 0.5f * v2;
}

vec3 GetMiddlePoint(vec3 v1, vec3 v2, vec3 v3){
    return (v1 + v2 + v3) / 3.0f;
}

void main()
{   
    //Pass along the values to the tessellation evaluation shader.
    positions[gl_InvocationID] = vertexPositions[gl_InvocationID];
    texCoords[gl_InvocationID] = vertexTexCoords[gl_InvocationID];
    normals[gl_InvocationID] = vertexNormals[gl_InvocationID];

    
 
    //Calculate the tessellation levels, only need to calculate once per triangle
    if (gl_InvocationID == 0)
    {
        vec3 position1 = vertexPositions[0];
        vec3 position2 = vertexPositions[1];
        vec3 position3 = vertexPositions[2];
 
        vec3 middlePoint1 = GetMiddlePoint(position2, position3);
        vec3 middlePoint2 = GetMiddlePoint(position3, position1);
        vec3 middlePoint3 = GetMiddlePoint(position1, position2);
 
        vec3 middleOfTriangle = GetMiddlePoint(middlePoint1, middlePoint2, middlePoint3);
 
        gl_TessLevelInner[0] = 0;
        gl_TessLevelOuter[0] = 0;
        gl_TessLevelOuter[1] = 0;
        gl_TessLevelOuter[2] = 0;
    }
}