#version 330

out vec4 FragColor;

// These definitions agree with the ObjectIds enum in scene.h
const int     nullId	= 0;
const int     skyId	= 1;
const int     seaId	= 2;
const int     groundId	= 3;
const int     roomId	= 4;
const int     boxId	= 5;
const int     frameId	= 6;
const int     lPicId	= 7;
const int     rPicId	= 8;
const int     teapotId	= 9;
const int     spheresId	= 10;
const int     floorId	= 11;

in vec3 normalVec, lightVec, eyeVec;
in vec2 texCoord;
in vec4 shadowCoord;
in vec3 tanVec;

uniform int objectId;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;

uniform vec3 Light;
uniform vec3 Ambient;

//uniform sampler2D shadowMap;
uniform sampler2D tex;
uniform sampler2D normalMap;
uniform sampler2D skyTex;
uniform sampler2D irrTex;
uniform float e;


// define PI
#define PI 3.1415926538

vec3 LightingPixel() {
    vec3 ret;
    vec3 N = normalize(normalVec);
    vec3 L = normalize(lightVec);
    vec3 V = normalize(eyeVec);
    vec3 H = normalize(L + V);
    vec3 T = normalize(tanVec);
    vec3 B = normalize(cross(T,N));

    vec3 Kd = diffuse;   
    vec3 Ks = specular;
    vec3 Ii = Light;
    float alpha = shininess;

    vec3 BRDF;
    vec3 F;
    float D;
    float G1 = 0.0;
    float G2 = 0.0;
    float LdotN;
    float HdotN;
    float VdotN = dot(V, N);
    float offset = 0.01;

    vec3 R;
    vec3 tmp;
    vec2 tex_uv = texCoord;
    vec3 delta;


    LdotN = max(dot(L, N), 0.0);
    HdotN = max(dot(H, N), 0.0);
    VdotN = dot(V, N);

    /*if (objectId==groundId || objectId==floorId || objectId==seaId) {
        ivec2 uv = ivec2(floor(100.0*texCoord));
        if ((uv[0]+uv[1])%2==0)
            Kd *= 0.9; }*/
    
    F = Ks + (1 - Ks) * pow((1 - dot(L,H)), 5);
    D = (alpha + 2) / (2 * PI) * pow(HdotN, alpha);

    alpha =  pow(alpha / 2.0 + 1.0, 0.5)  / (pow(1.0 - LdotN * LdotN, 0.5) / LdotN);
    if(alpha != 0 && alpha < 1.6)
        G1 = 3.535 * alpha + 2.181 * alpha * alpha / (1.0 + 2.276 * alpha + 2.577 * alpha * alpha);
    else
        G1 = 1.0;

    alpha = shininess;
    alpha = pow(alpha / 2.0 + 1.0, 0.5) / (pow(1.0 - VdotN * VdotN, 0.5) / VdotN);
    if(alpha != 0 && alpha < 1.6)
        G2 = 3.535 * alpha + 2.181 * alpha * alpha / (1.0 + 2.276 * alpha + 2.577 * alpha * alpha);
    else
        G2 = 1.0;

    if(G1 > 1.0) G1 = 1.0;
    if(G2 > 1.0) G2 = 1.0;

    BRDF = Kd * (1 / PI) +  F * G1 * G2 * D / (4 * LdotN * VdotN);


    ret = Ambient * Kd + Light * LdotN * BRDF;

    return ret;
}