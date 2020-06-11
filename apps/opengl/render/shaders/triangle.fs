/*
 * Original shader from: https://www.shadertoy.com/view/3td3DB
 */

#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

// glslsandbox uniforms
uniform float time;
uniform vec2 resolution;
uniform sampler2D texture1;

// shadertoy emulation
#define iTime time
#define iResolution resolution
// Blobs by @paulofalcao

#define time iTime

float makePoint(float x,float y,float fx,float fy,float sx,float sy,float t){
   float xx=x+sin(t*fx)*sx;
   float yy=y+cos(t*fy)*sy;
   return 1.0/sqrt(xx*xx+yy*yy);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {

   vec2 p=(fragCoord.xy/iResolution.x)*2.0-vec2(1.0,iResolution.y/iResolution.x);

   p=p*2.0;
   
   float x=p.x;
   float y=p.y;

   float a=
       makePoint(x,y,3.3,2.9,0.3,0.3,time);
   a=a+makePoint(x,y,1.9,2.0,0.4,0.4,time);
   a=a+makePoint(x,y,0.8,0.7,0.4,0.5,time);
   a=a+makePoint(x,y,2.3,0.1,0.6,0.3,time);
   a=a+makePoint(x,y,0.8,1.7,0.5,0.4,time);
   a=a+makePoint(x,y,0.3,1.0,0.4,0.4,time);
   a=a+makePoint(x,y,1.4,1.7,0.4,0.5,time);
   a=a+makePoint(x,y,1.3,2.1,0.6,0.3,time);
   a=a+makePoint(x,y,1.8,1.7,0.5,0.4,time);   
   
   float b=
       makePoint(x,y,1.2,1.9,0.3,0.3,time);
   b=b+makePoint(x,y,0.7,2.7,0.4,0.4,time);
   b=b+makePoint(x,y,1.4,0.6,0.4,0.5,time);
   b=b+makePoint(x,y,2.6,0.4,0.6,0.3,time);
   b=b+makePoint(x,y,0.7,1.4,0.5,0.4,time);
   b=b+makePoint(x,y,0.7,1.7,0.4,0.4,time);
   b=b+makePoint(x,y,0.8,0.5,0.4,0.5,time);
   b=b+makePoint(x,y,1.4,0.9,0.6,0.3,time);
   b=b+makePoint(x,y,0.7,1.3,0.5,0.4,time);

   float c=
       makePoint(x,y,3.7,0.3,0.3,0.3,time);
   c=c+makePoint(x,y,1.9,1.3,0.4,0.4,time);
   c=c+makePoint(x,y,0.8,0.9,0.4,0.5,time);
   c=c+makePoint(x,y,1.2,1.7,0.6,0.3,time);
   c=c+makePoint(x,y,0.3,0.6,0.5,0.4,time);
   c=c+makePoint(x,y,0.3,0.3,0.4,0.4,time);
   c=c+makePoint(x,y,1.4,0.8,0.4,0.5,time);
   c=c+makePoint(x,y,0.2,0.6,0.6,0.3,time);
   c=c+makePoint(x,y,1.3,0.5,0.5,0.4,time);
   
   vec3 d=vec3(a,b,c)/32.0;
   
   float ap = (d.x+d.y+d.z)/3.0;
   fragColor = (texture(texture1, TexCoord) + vec4(d.x,d.y,d.z,ap)) * 0.5;
}

void main( void ) {
    mainImage(FragColor, vec2(gl_FragCoord.x, gl_FragCoord.y));
}