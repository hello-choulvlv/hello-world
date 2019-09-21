uniform 	mat4 u_modelMatrix;
attribute 	vec4 a_position;
attribute   vec3 a_normal;
attribute   vec2 a_fragCoord;

varying 	vec3 v_normal;
varying 	vec2 v_fragCoord;

void main()
{
	vec4 _position = u_modelMatrix * vec4(a_position.xy,0.0,1.0);
	gl_Position = CC_PMatrix * _position;

	v_normal = vec3(a_normal.xy,1.0);
	v_fragCoord = a_fragCoord;
}