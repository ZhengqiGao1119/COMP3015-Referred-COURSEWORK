#version 460
out vec4 FragColor;

in vec3 Normal;
in vec2 Texcoord;
in vec3 FragPos;
in vec4 shadowCoord;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;
uniform vec3 matAmbient;
uniform vec3 matDiffuse;
uniform vec3 matSpecular;
uniform float matShininess;
uniform vec3 viewPos;  

float roughness;  
float metallic;  

uniform int useTexture;

uniform sampler2D texBaseColor;
uniform sampler2D texNormal;
uniform sampler2D texEmissive;
uniform sampler2D texMetallicRoughness;
uniform sampler2D texTransmission;
uniform sampler2DShadow shadowTex;

#define PI 3.141592653

vec3 diffColor;

vec3 calcF(float lDotH) {
    vec3 fo = mix(vec3(0.04), diffColor, metallic);
    return fo + (1 - fo) * pow(1.0 - lDotH, 5);
}
float GGX(float nDotH) {
    float alpha2 = pow(roughness, 4);
    float d = (nDotH * nDotH) * (alpha2 - 1) + 1;
    return step(0, nDotH) * alpha2 / (PI * d * d);
}
float G1(float dotProd) {
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float denom = dotProd * (1 - k) + k;
    return 1.0 / denom;
}

vec3 BRDF() {
    vec3 diffuseBrdf = vec3(0.0);  // Metallicif( !Material.Metal )f
    diffuseBrdf = diffColor;
    vec3 l = lightPosition - FragPos;

    float dist = length(l);
    l = normalize(l);
    vec3 lightI = lightColor / (dist * dist);
    vec3 v = normalize(viewPos - FragPos);
    vec3 h = normalize(v + l);
    float nDotH = dot(Normal, h);
    float lDotH = dot(l, h);
    float nDotL = max(dot(Normal, l), 0.0);
    float nDotV = dot(Normal, v);
    vec3 specBrdf = 0.25 * GGX(nDotH) * calcF(lDotH) * G1(nDotL) * G1(nDotV);
    return (diffuseBrdf + PI * specBrdf) * lightI * nDotL;
}

void main() {
    metallic=0.;
    roughness=.9;
    vec3 emissive;
    float alpha=1.;
    if(bool(useTexture))
    {
        vec2 a = texture(texMetallicRoughness, Texcoord).rg;
        metallic = a.x;
        roughness = a.y;

        emissive=texture(texEmissive,Texcoord).rgb;
        alpha = texture(texTransmission, Texcoord).r;
        if (alpha < 0.01) alpha = 1.;
    }
    // FragColor=vec4(Normal,1);
    // return;
    //PCF
    vec3 shadowP = shadowCoord.xyz / shadowCoord.w * 0.5 + 0.5;
    float shadow = 1.;
    if (shadowCoord.w > 0) {
        float sum = 0.;
        sum += textureOffset(shadowTex, shadowP, ivec2(-1, -1));
        sum += textureOffset(shadowTex, shadowP, ivec2(-1, 1));
        sum += textureOffset(shadowTex, shadowP, ivec2(1, -1));
        sum += textureOffset(shadowTex, shadowP, ivec2(1, 1));
        shadow = sum / 4;
    }

    diffColor = texture(texBaseColor, Texcoord).rgb;

    // ambient
    vec3 ambient = lightAmbient * matAmbient * diffColor;
  	
    vec3 col = BRDF();
    vec3 result = ambient + shadow * col + emissive;
    // vec3 result =  col;
    FragColor = vec4(result, alpha);
}