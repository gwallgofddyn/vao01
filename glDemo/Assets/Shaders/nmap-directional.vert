#version 410

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

// Directional light model (dont't need colour vector in vertex shader)
uniform vec3 lightDirection;

layout (location=0) in vec3 vertexPos;
layout (location=2) in vec3 vertexTexCoord;
layout (location=3) in vec3 vertexNormal;
layout (location=4) in vec3 tangent;
layout (location=5) in vec3 bitangent;

out SimplePacket {

    vec3 surfaceWorldPos;
    vec3 tsLightDirection; // normals will come from the normal map - we interpolate light direction vec in surface tangent space
    vec2 texCoord;

} outputVertex;


void main(void) {

	outputVertex.texCoord = vertexTexCoord.st;

    // transform normal vector by inverse-transpose of the model matrix

    // transform tanget space into world coords
    mat4 normalMatrix = transpose(inverse(modelMatrix));
    vec3 n = (normalMatrix * vec4(vertexNormal, 0.0)).xyz;
    vec3 t = (normalMatrix * vec4(tangent, 0.0)).xyz;
    vec3 b = (normalMatrix * vec4(bitangent, 0.0)).xyz;

    // map light direction into tangent space
    vec3 tVec;
	tVec.x = dot(lightDirection, t);
	tVec.y = dot(lightDirection, b);
	tVec.z = dot(lightDirection, n);
	outputVertex.tsLightDirection = normalize(tVec);

    //outputVertex.surfaceNormal = (transpose(inverse(modelMatrix)) * vec4(vertexNormal, 0.0)).xyz;

    // take vertexPos into world coords and pass onto fragment shader
    vec4 worldCoord = modelMatrix * vec4(vertexPos, 1.0);
    outputVertex.surfaceWorldPos = worldCoord.xyz; // don't need w element

    // take worldCoord rest of the way into clip coords and set in gl_Position
	gl_Position = projMatrix * viewMatrix * worldCoord;
}
