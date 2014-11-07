//attributes from the vertex buffer
attribute vec3 vertex;
attribute vec3 normal;
attribute vec2 uv;

varying vec2 uv_;
varying vec3 normal_;

void main(void) {
 // compute position
 gl_Position = modelToProjection * pos;

 uv_ = uv;
 // compute light info
  vec3 tnormal = (modelToCamera * vec4(normal, 0.0)).xyz;
  normal_ = tnormal;
  

 


} 