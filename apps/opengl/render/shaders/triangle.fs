/*
 * Original shader from: https://www.shadertoy.com/view/3td3DB
 */

#ifdef GL_ES
precision mediump float;
#endif

// glslsandbox uniforms
uniform float time;
uniform vec2 resolution;

// shadertoy emulation
#define iTime time
#define iResolution resolution

// --------[ Original ShaderToy begins here ]---------- //
// by @etiennejcb
// Using code from : https://www.shadertoy.com/view/Xt2XDt
// and 3D Simplex noise from https://www.shadertoy.com/view/XsX3zB
// Got help from @tdhooper

const float MAX_TRACE_DISTANCE = 10.0;           // max trace distance
const float INTERSECTION_PRECISION = 0.0006;        // precision of the intersection
const int NUM_OF_TRACE_STEPS = 200;


vec3 random3(vec3 c) {
	float j = 4096.0*sin(dot(c,vec3(17.0, 59.4, 15.0)));
	vec3 r;
	r.z = fract(512.0*j);
	j *= .125;
	r.x = fract(512.0*j);
	j *= .125;
	r.y = fract(512.0*j);
	return r-0.5;
}

/* skew constants for 3d simplex functions */
const float F3 =  0.3333333;
const float G3 =  0.1666667;

/* 3d simplex noise */
float simplex3d(vec3 p) {
	 /* 1. find current tetrahedron T and it's four vertices */
	 /* s, s+i1, s+i2, s+1.0 - absolute skewed (integer) coordinates of T vertices */
	 /* x, x1, x2, x3 - unskewed coordinates of p relative to each of T vertices*/
	 
	 /* calculate s and x */
	 vec3 s = floor(p + dot(p, vec3(F3)));
	 vec3 x = p - s + dot(s, vec3(G3));
	 
	 /* calculate i1 and i2 */
	 vec3 e = step(vec3(0.0), x - x.yzx);
	 vec3 i1 = e*(1.0 - e.zxy);
	 vec3 i2 = 1.0 - e.zxy*(1.0 - e);
	 	
	 /* x1, x2, x3 */
	 vec3 x1 = x - i1 + G3;
	 vec3 x2 = x - i2 + 2.0*G3;
	 vec3 x3 = x - 1.0 + 3.0*G3;
	 
	 /* 2. find four surflets and store them in d */
	 vec4 w, d;
	 
	 /* calculate surflet weights */
	 w.x = dot(x, x);
	 w.y = dot(x1, x1);
	 w.z = dot(x2, x2);
	 w.w = dot(x3, x3);
	 
	 /* w fades from 0.6 at the center of the surflet to 0.0 at the margin */
	 w = max(0.6 - w, 0.0);
	 
	 /* calculate surflet components */
	 d.x = dot(random3(s), x);
	 d.y = dot(random3(s + i1), x1);
	 d.z = dot(random3(s + i2), x2);
	 d.w = dot(random3(s + 1.0), x3);
	 
	 /* multiply d by w^4 */
	 w *= w;
	 w *= w;
	 d *= w;
	 
	 /* 3. return the sum of the four surflets */
	 return dot(d, vec4(52.0));
}

//----
// Camera Stuffs
//----
mat3 calcLookAtMatrix( in vec3 ro, in vec3 ta, in float roll )
{
    vec3 ww = normalize( ta - ro );
    vec3 uu = normalize( cross(ww,vec3(sin(roll),cos(roll),0.0) ) );
    vec3 vv = normalize( cross(uu,ww));
    return mat3( uu, vv, ww );
}

// checks to see which intersection is closer
// and makes the y of the vec2 be the proper id
vec2 opU( vec2 d1, vec2 d2 ){
    
	return (d1.x<d2.x) ? d1 : d2;
    
}

float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) +
         length(max(d,0.0));
}

float TWO_PI = 2.0*3.141592;

float sdSphere( vec3 p, float s )
{
  float angle = atan(p.z/p.x);
  float rad = 1.3;
  float offset = 1.0*angle/3.141592-2.5*p.y;
  vec3 inp = vec3(rad*cos(TWO_PI*(iTime-offset)),rad*sin(TWO_PI*(iTime-offset)),3.0*p.y);
  float ns =  simplex3d(inp);
  float sph = abs(length(p)-s)-0.01;
  float offset2 = 0.15; 
  if (sph < offset2) {
    sph += 0.5*offset2 * ns;
  }
  return sph;
}


//--------------------------------
// Modelling 
//--------------------------------
vec2 map( vec3 pos ){  

    return vec2( sdSphere( pos - vec3( .0 , .0 , -0.4 ) , 0.75 ) , 1. ); ;
    
}

vec3 result;

vec2 calcIntersection( in vec3 ro, in vec3 rd ){
    float h =  INTERSECTION_PRECISION*2.0;
    float t = 0.0;
	float res = -1.0;
    float id = -1.;
    
    for( int i=0; i< NUM_OF_TRACE_STEPS ; i++ ){
        
        if(t > MAX_TRACE_DISTANCE ) break;
	   	vec2 m = map( ro+rd*t );
        h = m.x;
        t += max(abs(h), 5.*INTERSECTION_PRECISION);
        id = m.y;
        result += vec3(0.0025);
    }

    if( t < MAX_TRACE_DISTANCE ) res = t;
    if( t > MAX_TRACE_DISTANCE ) id =-1.0;
    
    return vec2( res , id );
    
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    result = vec3(0.);
    vec3 color = vec3(0.,0.,0.);
    for(float i=-0.25;i<=0.25;i+=0.5){
        for(float j=-0.25;j<=0.25;j+=0.5){
            vec2 p = (-iResolution.xy + 2.0*(fragCoord.xy+vec2(i,j)))/iResolution.y;

            vec3 ro = vec3( 0., 0., 2.);
            vec3 ta = vec3( 0. , 0. , 0. );

            mat3 camMat = calcLookAtMatrix( ro, ta, 0.0 );  // 0.0 is the camera roll

            vec3 rd = normalize( camMat * vec3(p.xy,2.0) ); // 2.0 is the lens length

            vec2 res = calcIntersection( ro , rd  );
        }
    }
    fragColor = vec4(result,1.0);
}

// --------[ Original ShaderToy ends here ]---------- //

void main(void)
{
    mainImage(gl_FragColor, gl_FragCoord.xy);
}