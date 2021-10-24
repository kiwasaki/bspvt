// fragment shader
#version 410

in  vec3 color;
in  vec2 texcoord;
out vec4 frag_color;

uniform sampler2D tex0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
void main()
{
//    frag_color = vec4( 1.f, 1.f, 1.f, 1.f );
    frag_color = texture( tex0, texcoord ) * vec4( color, 1.f );
}

