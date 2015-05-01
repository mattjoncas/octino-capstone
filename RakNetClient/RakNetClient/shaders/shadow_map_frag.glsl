#version 440

in vec3 vertexWorld;
in vec3 camera_pos;

out float gl_FragDepth;
out vec4 fColor;

void main(){

    gl_FragDepth = (length(camera_pos - vertexWorld) - 0.1) / (200.0 - 0.1);
}