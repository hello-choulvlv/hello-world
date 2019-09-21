#version 330 core
//precision highp float;
layout(location=0)in      vec4        a_position;
layout(location=1)in      vec2        a_texCoord;
uniform   mat4    u_mvpMatrix;
uniform   vec3    u_originPosition;
uniform   vec3    u_totalAngle;
uniform   float   u_spanWidth;
uniform   float   u_frameAngle;

out     vec2      v_texCoord;

void    main()
{
     float     _angleX = u_frameAngle + (a_position.x - u_originPosition.x)/u_spanWidth*u_totalAngle.x;
     float     _angleZ = (a_position.z - u_originPosition.z)/u_spanWidth*u_totalAngle.z;

     float     _newY = sin(_angleX - _angleZ);
     _newY=sign(_newY)*_newY*_newY*0.17;

     gl_Position = u_mvpMatrix *vec4(a_position.x,a_position.y+ _newY,a_position.z,1.0);
     v_texCoord = a_texCoord;
}