R"zzz(#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D screenTexture;
const float offset = 1.0 / 300;

const vec4 outline_color = vec4(0.0, 0.0, 0.0, 1.0);
const float outline_size = 1.0f;

void main()
{
	vec2 offsets[9] = vec2[](
        vec2(-offset, offset),  // top-left
        vec2(0.0f,    offset),  // top-center
        vec2(offset,  offset),  // top-right
        vec2(-offset, 0.0f),    // center-left
        vec2(0.0f,    0.0f),    // center-center
        vec2(offset,  0.0f),    // center-right
        vec2(-offset, -offset), // bottom-left
        vec2(0.0f,    -offset), // bottom-center
        vec2(offset,  -offset)  // bottom-right    
    );

    mat3 sobelx = mat3(
    	1, 0, -1,
    	2, 0, -2,
    	1, 0, -1
    );

    mat3 sobely = mat3(
    	1, 2, 1,
    	0, 0, 0,
    	-1, -2, -1
    );

    vec3 sampleTex[9];
    vec3 grayTex[9];
    for (int i = 0; i < 3; ++i) {
    	sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
        vec3 gray = sampleTex[i];
        float average = 0.2126 * gray.r + 0.7152 * gray.g + 0.0722 * gray.b;
        grayTex[i] = vec3(average, average, average);
    }

    vec3 sum1 = vec3(0.0, 0.0, 0.0);
    vec3 sum2 = vec3(0.0, 0.0, 0.0);

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            vec3 gx = sobelx[i][j] * grayTex[i*3 + j];
            vec3 gy = sobely[j][i] * grayTex[i*3 + j];
            
            // sum1 += gx;
            sum2 += gy;
        }
    }

    float gr = sqrt(sum1.r * sum1.r) + (sum2.r * sum2.r);
    float gg = sqrt(sum1.g * sum1.g) + (sum2.g * sum2.g);
    float gb = sqrt(sum1.b * sum1.b) + (sum2.b * sum2.b);

    color = vec4(gr, gg, gb, 1.0);


}
)zzz"