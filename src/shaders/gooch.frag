R"zzz(
#version 330 core
in vec4 face_normal;
in vec4 vertex_normal;
in vec4 light_direction;
in vec4 camera_direction;
in vec2 uv_coords;
uniform vec3 control;
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
	if(control.y > 0.5){
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
			//fragment_color = vec4(1.0,1.0,1.0,alpha);
		}
	}else{
		if (length(texcolor) == 0.0) {
			//vec3 color = vec3(0.0, 1.0, 0.0);
			//vec3 color = vec3(diffuse);
			vec3 color = vec3(diffuse);
			//vec2 randuv = vec2(rand(light_direction.xy), rand(light_direction.zw));
			//vec3 color = vec3(diffuse) + texture(textureSampler, randuv).xyz;
			//vec3 color = texture(textureSampler, randuv).xyz;
			//vec3 color = vec3(diffuse) + vec3(randuv.x, randuv.y, 1.0);
			float dot_nl = dot(normalize(light_direction), normalize(vertex_normal));
			dot_nl = clamp(dot_nl, 0.0, 1.0);
			vec4 spec = specular * pow(max(0.0, dot(reflect(-light_direction, vertex_normal), camera_direction)), shininess);
			color = clamp(dot_nl * color + vec3(ambient) + vec3(spec), 0.0, 1.0);
			fragment_color = vec4(color, alpha);
		} else {
			fragment_color = vec4(texcolor.rgb, alpha);
		}
	}
	

	//fragment_color = garlic;
}
)zzz"
