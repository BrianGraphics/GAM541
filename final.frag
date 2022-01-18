/////////////////////////////////////////////////////////////////////////
// Pixel shader for lighting
////////////////////////////////////////////////////////////////////////
#version 330

out vec4 FragColor;

in vec3 normalVec, eyeVec;

vec3 LightingPixel();

//uniform int reflective;
uniform sampler2D Top;
uniform sampler2D Bot;

void main()
{
	/*if(bool(reflective)) {
		vec3 N = normalize(normalVec);
		vec3 V = normalize(eyeVec);

		vec2 uv;
		vec3 ret;
		vec3 R = 2 * dot(V, N) * N - V;
		R = normalize(R);
	
		if(R.z > 0)
			uv = vec2(R.x / (1 + R.z), R.y / (1 + R.z));
		else
			uv = vec2(R.x / (1 - R.z), R.y / (1 - R.z));

		uv = uv * 0.5 + vec2(0.5, 0.5);

		if(R.z > 0)
			ret = texture2D(Top, uv).xyz;
		else
			ret = texture2D(Bot, uv).xyz;
    
        FragColor.xyz = ret;
        return;
    }*/

	FragColor.xyz = LightingPixel();  
}
