#version 440

in vec3 position;
in vec3 normal;
in vec2 texcoord;
in vec3 color;

uniform	mat4 model;
uniform mat3 norm;

uniform GlobalMatrices {
	mat4 view;
	mat4 proj;
	vec3 camera_position;
};

out vec3 Color;
out vec2 Texcoord;

//lighting
out vec3 normalWorld;
out vec3 vertexWorld;
out vec3 camera_pos;

void main() {
    Texcoord = texcoord;
    gl_Position = proj * view * model * vec4(position, 1.0);
    
    // Transform vertex position into eye coordinates
    vertexWorld = (model * vec4(position, 1.0)).xyz;
    
    //use normals
    normalWorld = normalize(norm * normal);
    
    camera_pos = camera_position;

    Color = color;
}