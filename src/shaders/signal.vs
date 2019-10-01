#version 330 core
layout (location = 0) in float yPos;
//out vec2 TexCoord;

//uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;
uniform float dx;

void main()
{
    //gl_Position = projection * view * model * vec4(aPos, 1.0);
    gl_Position = vec4(gl_VertexID * dx, yPos, 0.0, 1.0);
}