
#version 150


in vec3 color_frag;

void main(void)
{
    vec3 c;
    c.r = abs(color_frag.r);
    c.g = abs(color_frag.g);
    c.b = abs(color_frag.b);
    gl_FragColor = vec4(c, 1.0);
}
