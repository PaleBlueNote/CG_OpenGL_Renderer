#version 410 core

out vec4 out_Color;

uniform vec3 lightPos = vec3(10,20,10);
uniform vec3 lightIntensity = vec3(1,1,1);
uniform vec3 diffColor = vec3(1,0.5,0);
uniform vec3 cameraPos = vec3(0,0,5);
uniform float shininess = 250;
uniform sampler2D diffTex;
uniform sampler2D bumpTex;

const float TEX_DELTA = 0.0001;

in vec3 worldPos;
in vec2 texCoord;
in vec3 normal;

mat3 getTBN(vec3 N) {
    vec3 Q1 = dFdx(worldPos);
    vec3 Q2 = dFdy(worldPos);
    vec2 st1 = dFdx(texCoord);
    vec2 st2 = dFdy(texCoord);

    float D = st1.s * st2.t - st2.s * st1.t;
    
    return mat3(normalize((Q1 * st2.t - Q2 * st1.t) * D),
                normalize((-Q1 * st2.s + Q2 * st1.s) * D),
                N);
}

void main(void)
{
	vec3 N = normalize(normal);

    mat3 TBN = getTBN(N);
    float Bu = texture(bumpTex, texCoord + vec2(TEX_DELTA, 0)).r 
             - texture(bumpTex, texCoord - vec2(TEX_DELTA, 0)).r;
             
    float Bv = texture(bumpTex, texCoord + vec2(0, TEX_DELTA)).r 
             - texture(bumpTex, texCoord - vec2(0, TEX_DELTA)).r;
    
    vec3 bumpVec = vec3(-Bu * 15.0, -Bv * 15.0, 1.0);
    N = normalize(TBN * bumpVec);


	vec3 L = normalize(lightPos-worldPos);
	vec3 V = normalize(cameraPos-worldPos);
	vec3 diffuseColor = diffColor;
	diffuseColor = texture(diffTex,texCoord).rgb;
	vec3 color = diffuseColor * lightIntensity * max(0,dot(N,L));
	vec3 R = reflect(-L,N);
	color += lightIntensity * pow(max(0,dot(R,V)),shininess);
	out_Color = vec4(pow(color,vec3(1/2.2)), 1.0);
}