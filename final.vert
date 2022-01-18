/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 WorldView, WorldInverse, WorldProj, ModelTr, NormalTr;
uniform mat4 Camera;
uniform vec3 lightPos, teapotPos;
uniform mat4 ShadowMatrix;
uniform int reflectPass, UPorDown;

in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;

out vec3 normalVec, lightVec, eyeVec;
out vec2 texCoord;
out vec4 shadowCoord;

void LightingPixel(vec3 eye);

void main()
{      
    vec3 eyePos;

    //gl_Position = WorldProj*WorldView*ModelTr*vertex;  
    gl_Position = Camera*ModelTr*vertex;  
    eyePos = (WorldInverse*vec4(0,0,0,1)).xyz;
    LightingPixel(eyePos);
}
