R"zzz(#version 330 core
in vec2 TexCoords;
out vec4 color;
uniform vec3 control;
uniform sampler2D screenTexture;
const float offset = 1.0 / 900;

const vec4 outline_color = vec4(0.0, 0.0, 0.0, 1.0);
const float outline_size = 1.0f;
const float threshold = 0.999;

void main()
{
    bool showOutline = control.x > 0.5;
    if(!showOutline)
        color = texture(screenTexture, TexCoords);
    else{
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
             1,  2,  1,
             0,  0,  0,
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

        vec3 x_sum = vec3(0.0, 0.0, 0.0);
        vec3 y_sum = vec3(0.0, 0.0, 0.0);

        for (int i = 0; i < 9; ++i) {
    //        vec3 gx = sobelx[i] * grayTex[i];
    //        vec3 gy = sobely[i] * grayTex[i];
            vec3 gx = sobelx[i] * sampleTex[i];
            vec3 gy = sobely[i] * sampleTex[i];

            x_sum += gx;
            y_sum += gy;
        }

        vec3 mag = sqrt( pow(x_sum, vec3(2)) + pow(y_sum, vec3(2)) );

        float gmag = length(mag);

        color = vec4(vec3(gmag), 1.0);
        // color = vec4(mag, 1.0);

      
        bool isEdge = mag.r > threshold || mag.g > threshold || mag.b > threshold;
        if (isEdge)
            color = outline_color;
        else 
            color = texture(screenTexture, TexCoords);

    }
	
}
)zzz"