#version 440

in vec3 Color;
in vec2 Texcoord;
//lighting
in vec3 normalWorld;
in vec3 vertexWorld;
in vec3 camera_pos;

layout(binding=0) uniform sampler2D tex;
layout(binding=1) uniform samplerCubeShadow u_shadowCubeMap;

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

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1), 
   vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
   vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
   vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

// function compares distance from shadow map with current distance
void sampleShadowMap(in vec3 baseDirection, in vec3 baseOffset, in float curDistance, inout float shadowFactor, inout float numSamples){
   shadowFactor += texture(u_shadowCubeMap, vec4(baseDirection + baseOffset, curDistance));
   numSamples += 1;
}


void main(){
    vec3 lcolor = vec3(0.0);
    for (int i = 0; i < num_lights; i++){
        vec4 diffuse_product = material_diffuse * lights[i].light_diffuse;
        vec4 specular_product = material_specular * lights[i].light_specular;
        vec4 ambient_product = material_ambient * lights[i].light_ambient;
        
        float attenuation = 1.0f;
        float shadowFactor = 1.0f;
        vec3 lightVectorWorld;
        //point light
        if (lights[i].light_direction.xyz == vec3(0.0)){
            lightVectorWorld = normalize((lights[i].light_position).xyz - vertexWorld); //<-- this works when an object is rotated
            float distanceToLight = length((lights[i].light_position).xyz - vertexWorld);
            attenuation =  pow(max(0.0, 1.0 - (distanceToLight / lights[i].light_size)), lights[i].light_drop_off + 1.0);

            //shadow mapping if is point light
            float currentDistanceToLight = (distanceToLight - 0.1) / (200.0 - 0.1);
            currentDistanceToLight = clamp(currentDistanceToLight, 0.0, 1.0);
            vec3 fromLightToFragment = lightVectorWorld;
            // sample shadow cube map
            if (i == 0){
                shadowFactor = texture(u_shadowCubeMap, vec4(-fromLightToFragment, currentDistanceToLight));
            }
        }
        //directional light
        else{
            lightVectorWorld = normalize(lights[i].light_direction.xyz);
        }

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

        lcolor += ambient.xyz + shadowFactor * (attenuation * (clamp(diffuse.xyz, 0, 1) + clamp(specular.xyz, 0, 1)));
    }

    fColor = vec4(Color, 0.4) * vec4(lcolor, 1.0);
}