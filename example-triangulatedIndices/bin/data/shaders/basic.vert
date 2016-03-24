
#version 150

in vec3 vertex;
in vec3 color;
uniform mat4 matrix;


out vec3 color_frag;



void main(void){

   color_frag = color;
    gl_Position = matrix * vec4(vertex,1);
}
