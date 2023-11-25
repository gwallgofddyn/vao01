#version 410

// Diffuse texture - directional light

// Texture sampler (for diffuse surface colour)
uniform sampler2D diffuseTexture; // tex unit 0
uniform sampler2D normalMapTexture; // tex unit 1


// Directional light model - colour component
uniform vec3 lightColour;


in SimplePacket {
	
	vec3 surfaceWorldPos;
	vec3 tsLightDirection;
	vec2 texCoord;

} inputFragment;


layout (location=0) out vec4 fragColour;

void main(void) {

	// Get normal from normal map
	vec3 N = texture2D(normalMapTexture, inputFragment.texCoord).xyz;
	N = vec3(2.0) * (N - vec3(0.5));
	N = normalize(N);
	
	// Normalise light direction in tangent space
	vec3 L = normalize(inputFragment.tsLightDirection);
	// calculate lambertian (l)
	float l = dot(N, L);

	// Calculate diffuse brightness / colour for fragment
	vec4 surfaceColour = texture2D(diffuseTexture, inputFragment.texCoord);
	vec3 diffuseColour = surfaceColour.rgb * lightColour * l;

	fragColour = vec4(diffuseColour, 1.0);
	//fragColour = vec4(vec3(l, l, l), 1.0);
}
