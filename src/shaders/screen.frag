R"zzz(#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D screenTexture;
const float offset = 1.0 / 300;

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

    float sobelx[9] = float[](
    	1, 0, -1,
    	2, 0, -2,
    	1, 0, -1
    );

    float sobely[9] = float[](
    	1, 2, 1,
    	0, 0, 0,
    	-1, -2, -1
    );

    vec3 sampleTex[9];
    vec3 grayTex[9];
    for (int i = 0; i < 9; ++i) {
    	sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
        vec3 gray = sampleTex[i];
        float average = 0.2126 * gray.r + 0.7152 * gray.g + 0.0722 * gray.b;
        grayTex[i] = vec3(average, average, average);
    }

    vec3 col = vec3(0.0);
    vec3 gx = vec3(0.0);
    vec3 gy = vec3(0.0);
    for (int i = 0; i < 9; ++i) {
    	gx += grayTex[i] * sobelx[i];
    	gy += grayTex[i] * sobely[i];
    	// col += sampleTex[i] * sobelx[i];
        col += sqrt(gx*gx + gy*gy);
        //col += grayTex[i] * sobely[i];
    }

	color = vec4(col, 1.0);

    // color = gray;
}
)zzz"