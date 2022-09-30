#version 330 core
out vec4 FragColor;
in vec2 vertexUV;

uniform sampler2D _MainTex;

void main()
{
    FragColor = texture(_MainTex, vertexUV);
}