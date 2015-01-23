#version 440

in vec3 position;
in vec3 normal;
in vec2 texcoord;
in vec3 color;

uniform	mat4 model;

uniform GlobalMatrices {
	mat4 view;
	mat4 proj;
	vec3 camera_position;
};

out vec3 Color;

void main() {
    gl_Position = proj * view * model * vec4(position, 1.0);

    Color = vec3(1.0, 0.0, 0.0);
}