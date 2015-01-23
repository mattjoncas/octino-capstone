#version 440

in vec3 Color;

out vec4 fColor;

void main(){
	fColor = vec4(Color, 1.0);
}