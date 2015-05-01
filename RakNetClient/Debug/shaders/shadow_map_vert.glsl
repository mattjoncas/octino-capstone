#version 440

in vec3 position;
in vec3 normal;
in vec2 texcoord;
in vec3 color;

uniform mat4 model;

uniform GlobalMatrices {
    mat4 view;
    mat4 proj;
    vec3 camera_position;
};

//lighting
out vec3 vertexWorld;
out vec3 camera_pos;

void main() {
    gl_Position = proj * view * model * vec4(position, 1.0);
    
    // Transform vertex position into eye coordinates
    vertexWorld = (model * vec4(position, 1.0)).xyz;
    
    camera_pos = camera_position;
}