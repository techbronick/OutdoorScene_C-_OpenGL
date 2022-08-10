// VERTEX SHADER
#version 330

#define MAX_BONES 142
uniform mat4 bones[MAX_BONES];

uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;
uniform float fogDensity;

 
in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoord;

in vec3 aTangent;
in vec3 aBiTangent;
in ivec4 aBoneId;
in vec4 aBoneWeight;

out vec2 texCoord0;
out vec2 texCoord1;
out vec3 normal;
//out vec3 texCoordCubeMap;
out vec4 color;
out vec4 position;
out mat3 matrixTangent;
out float fogFactor;





void main(void) 
{
	mat4 matrixBone;
	if (aBoneWeight[0] == 0.0)
		matrixBone = mat4(1);
	else
		matrixBone = (bones[aBoneId[0]] * aBoneWeight[0] +
					  bones[aBoneId[1]] * aBoneWeight[1] +
					  bones[aBoneId[2]] * aBoneWeight[2] +
					  bones[aBoneId[3]] * aBoneWeight[3]);

	// calculate position
	position = matrixModelView *matrixBone* vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;

	// calculate normal
	normal=normalize(mat3(matrixModelView) * mat3(matrixBone) * aNormal);

	fogFactor = exp2(-fogDensity * length(position));

	//calculate tangentlocal system transformation
	vec3 tangent = normalize(mat3(matrixModelView) * aTangent);
	vec3 biTangent = normalize(mat3(matrixModelView) * aBiTangent);
	matrixTangent = mat3(tangent, biTangent, normal);

	// calculate textures coordinate
	texCoord0 = aTexCoord;

	//setting initial colour
	color = vec4(0, 0, 0, 1);
}
