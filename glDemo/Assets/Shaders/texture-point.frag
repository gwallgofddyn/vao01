#version 410

// Diffuse texture - point light

// Texture sampler (for diffuse surface colour)
uniform sampler2D texture;

// Point light model
uniform vec3 lightPos;
uniform vec3 lightColour;
uniform vec3 lightAttenuation; // x=constant, y=linear, z=quadratic


in SimplePacket {
	
	vec3 surfaceWorldPos;
	vec3 surfaceNormal;
	vec2 texCoord;

} inputFragment;


layout (location=0) out vec4 fragColour;

void main(void) {

	vec3 surfacetoLight = lightPos - inputFragment.surfaceWorldPos;

	// calculate lambertian
	vec3 surfaceToLightNormalised = normalize(surfacetoLight);
	vec3 N = normalize(inputFragment.surfaceNormal);
	float l = dot(N, surfaceToLightNormalised);

	// calculate attenuation
	float d = length(surfacetoLight);

	float kc = 1.0;//lightAttenuation.x;
	float kl = 0.2;//lightAttenuation.y;
	float kq = 0.01;//lightAttenuation.z;

	float a = 1.0 / ( kc + (kl * d) + (kq * d * d) );

	// Calculate diffuse brightness / colour for fragment
	vec4 surfaceColour = texture2D(texture, inputFragment.texCoord);

	vec3 diffuseColour = surfaceColour.rgb * lightColour * l * a;

	fragColour = vec4(diffuseColour, surfaceColour.a);
}
