#version 460 core
in vec2 TexCoords;


out vec4 color;

layout (binding = 0) uniform sampler2D text;


uniform vec2 WindowSize;

void main()
{

    if(texture(text,TexCoords).r==1)
        color= vec4(1.0,1.0,1.0,1.0);
    else
        discard;

}  