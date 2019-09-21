#version 410
precision highp float;
uniform    sampler2D      u_globlePosition;
uniform    sampler2D      u_globleNormal;
uniform    sampler2D      u_noiseMap;
uniform    vec3           u_samples[64];
uniform    vec2           u_nearFarDistance;
uniform    vec2           u_noiseScale;
uniform    float          u_kernelRadius;
uniform    mat4           u_projMatrix;
layout(location=0)out     float      ssao_depth;

in      vec2      v_fragCoord;

void   main()
{
	vec3     _position = texture(u_globlePosition,v_fragCoord).xyz;
    vec3     _normal = texture(u_globleNormal,v_fragCoord).xyz;
    vec3     _randomVec =normalize( texture(u_noiseMap,v_fragCoord * u_noiseScale).xyz );
    vec3     _tantVec = normalize(_randomVec - _normal * dot(_normal,_randomVec));
    vec3     _binormalVec = cross(_normal,_tantVec);
    mat3     _tbnMatrix =mat3(_tantVec,_binormalVec,_normal);
    ssao_depth =0.0;
    const    int    _kernel_size =64;
    for(int i=0;i<_kernel_size;++i)
    {
    	vec3     _sample = _tbnMatrix * u_samples[i];
    	vec3     _other_pos = _position + _sample * u_kernelRadius;
    	vec4     _offset = u_projMatrix * vec4(_other_pos,1.0);
    	_offset.xyz/=_offset.w;
    	_offset.xyz=_offset.xyz * 0.5 +0.5;
    	float    _other_depth = texture(u_globlePosition,_offset.xy).z;
    	float    _fix_depth=smoothstep(0.0,1.0,u_kernelRadius/abs(_position.z - _other_depth));
    	ssao_depth +=float(_other_pos.z<_other_depth)*_fix_depth;
    }
    ssao_depth = 1.0 - ssao_depth/float(_kernel_size);
}

