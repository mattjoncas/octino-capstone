#version 440

in vec3 Color;
in vec2 Texcoord;
//lighting
in vec3 normalWorld;
in vec3 vertexWorld;
in vec3 camera_pos;

layout(binding=0) uniform sampler2D tex;

uniform MaterialUniforms {
    vec4 material_ambient, material_diffuse, material_specular;
    float shininess;
};

#define MAX_LIGHTS 10
uniform int num_lights;

struct Light {
    vec4 light_position, light_direction, light_ambient, light_diffuse, light_specular;
    float light_size, light_drop_off;
};

uniform LightingUniforms
{
  Light lights[MAX_LIGHTS];
};

out vec4 fColor;

void main(){
    /*
    vec4 diffuse_product = material_diffuse * lights[0].light_diffuse;
    vec4 specular_product = material_specular * lights[0].light_specular;
    vec4 ambient_product = material_ambient * lights[0].light_ambient;

    //vec4 texColor = texture(tex, Texcoord);

    vec3 lightVectorWorld = normalize((lights[0].light_position).xyz - vertexWorld); //<-- this works when an object is rotated
     //diffuse brightness
    float Kd = dot(lightVectorWorld, normalize(normalWorld));
    vec4 diffuse = Kd * diffuse_product;

    //spec
    vec3 reflected = reflect(-lightVectorWorld, normalize(normalWorld));
    vec3 eyeVectorWorld = normalize(camera_pos - vertexWorld);
    float s = dot(reflected, eyeVectorWorld);
    //specular brightness
    float Ks = pow(s, shininess);
    vec4 specular = Ks * specular_product;

    // Compute terms in the illumination equation
    vec4 ambient = ambient_product;

    vec3 lcolor = ambient.xyz + clamp(diffuse.xyz, 0, 1) + clamp(specular.xyz, 0, 1);
    //Color.a = 1.0;*/
	fColor = vec4(Color, 1.0);// * vec4(lcolor, 1.0);// * texColor;
}