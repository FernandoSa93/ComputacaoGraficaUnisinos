//Código fonte do Fragment Shader (em GLSL)
#version 400

in vec4 finalColor;
out vec4 color;

void main()
{
    color = finalColor;
}