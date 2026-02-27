#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;  // blurred bright areas
uniform sampler2D original;  // original scene
uniform vec2 texSize;

void main()
{
    vec2 texel = 1.0 / texSize;
    vec4 blur = vec4(0.0);

    // 9x9 box blur
    for (int x = -2; x <= 2; x++)
        for (int y = -2; y <= 2; y++)
            blur += texture(texture0, fragTexCoord + vec2(x, y) * texel);
    blur /= 16.0;

    vec4 orig = texture(original, fragTexCoord);
    finalColor = orig + blur * 1.5;
}