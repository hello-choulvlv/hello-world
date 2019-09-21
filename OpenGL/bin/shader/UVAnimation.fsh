#version 330 core
uniform sampler2D u_texture1;
uniform sampler2D u_lightTexture;   
uniform vec4 v_LightColor;
uniform vec2 v_animLight;     
varying vec2 v_texCoord;
varying vec3 v_normal;

void main(void) 
{
    vec4 lightcolor = texture2D(u_lightTexture, v_texCoord + v_animLight.xy) * v_LightColor;

    const float dotValue = normalize(v_normal).y;
    const float _light_factor = smoothstep(-0.5,1.0,dotValue);

	gl_FragColor = texture2D(u_texture1, v_texCoord) + lightcolor*_light_factor;
}
