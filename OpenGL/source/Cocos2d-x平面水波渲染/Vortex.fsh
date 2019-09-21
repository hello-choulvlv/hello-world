#ifdef OPENGL_ES
precision mediump float;
#endif

uniform sampler2D u_texture;
// Varyings
//varying vec2 v_texCoord;
#ifdef GL_ES
varying mediump vec2 v_texCoord;
#else
varying vec2 v_texCoord;
#endif
uniform	float u_radius;
uniform	float u_angle;

vec2 Vortex( vec2 uv )
{
	uv -= vec2(0.5, 0.5);
	float dist = length(uv);
	float percent = (u_radius - dist) / u_radius;
	float sign_flag = step(0.0,percent) * step(percent,1.0) ;//==> percent>=0.0 && percent <=1.0

	float theta = sign_flag * percent * percent * u_angle * 8.0;
	float s = sin(theta);
	float c = cos(theta);
	uv = vec2(dot(uv, vec2(c, -s)), dot(uv, vec2(s, c)));

	uv += vec2(0.5, 0.5);

	return uv;
}

void main()
{
	gl_FragColor = texture2D( u_texture,Vortex(v_texCoord) );
}