R"zzz(
#version 330 core
in vec4 face_normal;
in vec4 vertex_normal;
in vec4 light_direction;
in vec4 camera_direction;
in vec2 uv_coords;
uniform vec4 garlic;
uniform vec4 diffuse;
uniform vec4 ambient;
uniform vec4 specular;
uniform float shininess;
uniform float alpha;
uniform sampler2D textureSampler;
out vec4 fragment_color;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
void main() {
	vec3 texcolor = texture(textureSampler, uv_coords).xyz;
	if (length(texcolor) == 0.0) {
		float dot_nl = dot(normalize(light_direction), normalize(vertex_normal));
		vec3 kd = vec3(diffuse);
		vec3 kblue = vec3(0, 0, garlic.x);
		vec3 kyellow = vec3(garlic.y, garlic.y, 0);
		vec3 kcool = kblue + garlic.z*kd;
		vec3 kwarm = kyellow + garlic.w*kd;
		dot_nl = (1 + dot_nl)/2.0;
		fragment_color = vec4(dot_nl * kcool + (1-dot_nl) * kwarm, alpha);

	} else {
		fragment_color = vec4(texcolor.rgb, alpha);
	}

	//fragment_color = garlic;
}
)zzz"
