varying vec2 uv_;
varying vec3 normal_;


void main(void)
{
    vec3 diffuse;
    float specular;
    vec3 color = max(diffuse,_ambient.xyz)*vec3(1.0, 1.0, 0.0);
    
    gl_FragColor = normal_*uv_;
}