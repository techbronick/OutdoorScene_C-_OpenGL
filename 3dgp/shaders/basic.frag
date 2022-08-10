// FRAGMENT SHADER
#version 330

uniform mat4 matrixView;

uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;

uniform float shininess;
uniform float att_quadratic ;

uniform vec3 fogColour;
uniform sampler2D texture0;
uniform sampler2D textureNormal;

in vec2 texCoord0;
in vec3 normal;
in vec4 color;
in vec4 position;
in mat3 matrixTangent;
in float fogFactor;

out vec4 outColor;

vec3 normalNew;



struct AMBIENT
{	
	int on;
	vec3 color;
};
uniform AMBIENT lightAmbient,bulbAmbient;

//Calculate Ambient Light function

vec4 AmbientLight(AMBIENT light)
{
	return vec4(materialAmbient * light.color, 1);
}



struct DIRECTIONAL
{	
	int on;
	vec3 direction;
	vec3 diffuse;
};
uniform DIRECTIONAL lightDir1,lightDir2;

//Calculate Directional Light function

vec4 DirectionalLight(DIRECTIONAL light)
{
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize(mat3(matrixView) * light.direction);
	float NdotL = dot(normal, L);
	if (NdotL > 0)
	color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	return color;
}

//Directional light used for the normal maps
vec4 DirectionalLightNormal(DIRECTIONAL light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize(mat3(matrixView) * light.direction);
	float NdotL = dot(normalNew, L);
	if (NdotL > 0)
	color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	return color;
}



struct SPOT
{
	int on;
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	vec3 direction;
	float cutoff; 
	float attenuation;

};
uniform SPOT spotLight1,spotLight2,spotLight3,spotLight4,spotLight5;

//Calculate Spot Light function

vec4 SpotLight(SPOT light)
{
	vec4 color = vec4(0, 0, 0, 0);

	vec3 L = normalize((matrixView * vec4(light.position,1)-position).xyz);
	float NdotL = dot(L,normal);


	if(NdotL>0)
		color+=vec4(materialDiffuse*light.diffuse,1)*NdotL;

	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);

	if (NdotL > 0 && RdotV > 0)
	    color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	vec3 D = normalize(mat3(matrixView) * light.direction);    // normalizing direction
    float spotFactor= dot(-L,D);                               //calculating spot factor
    float a = acos(spotFactor);                                //calculating the angle
    float cutoff = radians(clamp(light.cutoff, 0, 90));        // converting cutoff to degrees

    if(a <= cutoff)
    spotFactor=pow(spotFactor,light.attenuation);

        if(a > cutoff)
        spotFactor = 0;

	float dist = length(matrixView * vec4(light.position, 1) - position);
	float att = 1 / (att_quadratic * pow(dist,2));
	return spotFactor*att* color;
}



void main(void) 
{
    outColor = color;

    normalNew= 2.0 * texture(textureNormal, texCoord0).xyz  -vec3(1.0, 1.0, 1.0); 

	normalNew= normalize(matrixTangent * normalNew);
	
	// calculate light
	if (lightAmbient.on == 1) 
		outColor += AmbientLight(lightAmbient);
	
	if (bulbAmbient.on == 1) 
		outColor += AmbientLight(bulbAmbient);

	if (lightDir1.on == 1) 
		outColor += DirectionalLight(lightDir1);

	if (lightDir2.on == 1) 
		outColor += DirectionalLightNormal(lightDir2);

	if (spotLight1.on == 1) 
		outColor += SpotLight(spotLight1);

	if (spotLight2.on == 1) 
		outColor += SpotLight(spotLight2);

	if (spotLight3.on == 1) 
		outColor += SpotLight(spotLight3);

	if (spotLight4.on == 1) 
		outColor += SpotLight(spotLight4);

	if (spotLight5.on == 1) 
		outColor += SpotLight(spotLight5);

	outColor *= texture(texture0, texCoord0);
	outColor = mix(vec4(fogColour, 1), outColor, fogFactor);

}
