#version 440

in vec3 Color;
in vec2 Texcoord;
//lighting
in vec3 normalWorld;
in vec3 vertexWorld;
in vec3 camera_pos;

layout(binding=0) uniform sampler2D tex;

layout(binding=2) uniform samplerCubeShadow u_shadowCubeMap;
layout(binding=3) uniform samplerCube cubeMap;

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
    vec3 lcolor = vec3(0.0);
    for (int i = 0; i < num_lights; i++){
        vec4 diffuse_product = material_diffuse * lights[i].light_diffuse;
        vec4 specular_product = material_specular * lights[i].light_specular;
        vec4 ambient_product = material_ambient * lights[i].light_ambient;
        
        float attenuation = 1.0f;
        float shadowFactor = 1.0f;
        vec3 lightVectorWorld;
        vec3 normal = normalize(normalWorld);
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
            if (i == 0 && lights[i].light_position.w == 1.0f){
                shadowFactor = texture(u_shadowCubeMap, vec4(-fromLightToFragment, currentDistanceToLight));
            }
        }
        //directional light
        else{
            lightVectorWorld = normalize(lights[i].light_direction.xyz);
        }

        //diffuse brightness
        float Kd = dot(lightVectorWorld, normal);
        vec4 diffuse = Kd * diffuse_product;

        //spec brightness
        vec3 viewDir = normalize(camera_pos - vertexWorld); //eyeVector
        vec3 halfDir = normalize(lightVectorWorld + viewDir); //reflected
        float specAngle = max(dot(halfDir, normal), 0.0);
        float Ks = pow(specAngle, shininess);
        
        vec4 specular = Ks * specular_product;

        // Compute terms in the illumination equation
        vec4 ambient = ambient_product;

        //lcolor += ambient.xyz + shadowFactor * (attenuation * (clamp(diffuse.xyz, 0, 1) + clamp(specular.xyz, 0, 1)));
        lcolor += clamp(specular.xyz, 0, 1); //only render spec
    }
    vec3 eyeDir = normalize(camera_pos - vertexWorld);
    vec4 cubeMapSample = texture(cubeMap, reflect(-eyeDir, normalize(normalWorld)));
    //cubeMapSample = texture(cubeMap, refract(eyeDir, normalize(normalWorld), 1.0 / 1.333));
    cubeMapSample = mix(cubeMapSample, texture(cubeMap, refract(eyeDir, normalize(normalWorld), 1 / 1.333)), pow(dot(normalize(normalWorld), eyeDir), 5));
    //vec4 cubeMapSample = texture(cubeMap, mix(reflect(eyeDir, normalWorld),refract(eyeDir, normalWorld, 1.333 / 1), pow(dot(normalWorld, eyeDir), 5)));
    //vec4 cubeMapSample = texture(cubeMap, normalize(camera_pos - vertexWorld)); //portal effect

	fColor = vec4(Color, 1.0) * cubeMapSample + vec4(lcolor, 1.0);
}