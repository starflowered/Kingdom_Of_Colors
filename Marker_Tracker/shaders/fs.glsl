#version 460 core
in vec2 TexCoords;


out vec4 color;

layout (binding = 0) uniform sampler2D text;
layout (location = 2) uniform vec4 colorText;


void main()
{

    if(texture(text,TexCoords).r==1)
        color= colorText;
    else
        discard;

}  