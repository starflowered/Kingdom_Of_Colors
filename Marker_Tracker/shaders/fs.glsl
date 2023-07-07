#version 460 core
in vec2 TexCoords;


out vec4 color;

//layout (binding = 0) uniform sampler2D text;
//layout (binding = 1) uniform sampler2D background_image;
uniform sampler2D background_image;

uniform vec2 WindowSize;

void main()
{
    //color = vec4(1.0,0.0,0.0,1.0);
   //color = vec4(texture(background_image, TexCoords).rgb,1.0);
    vec2 frag_coords = gl_FragCoord.xy/WindowSize;
    frag_coords = vec2(frag_coords.x, frag_coords.y);
    //color = vec4(0.0,0.0,0.0,1.0);
    color = vec4(texture(background_image,frag_coords).rgb,1.0);
    //if(texture(text,TexCoords).r==1)
     //   color= vec4(1.0,1.0,0.0,1.0);

     //color = vec4(1.0,1.0,0.0,1.0);


}  