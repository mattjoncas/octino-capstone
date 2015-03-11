#version 440

in vec3 Color;
in vec2 Texcoord;
//lighting
in vec3 normalWorld;
in vec3 vertexWorld;
in vec3 camera_pos;

layout(binding=0) uniform sampler2D tex;
layout(binding=1) uniform sampler2D normal_map;
layout(binding=2) uniform samplerCubeShadow u_shadowCubeMap;

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

//normal mapping
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
 
    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

vec3 perturb_normal( vec3 N, vec3 V, vec2 texcoord )
{
    // assume N, the interpolated vertex normal and 
    // V, the view vector (vertex to eye)
    vec3 map = texture(normal_map, texcoord ).xyz;
    map = map * 255./127. - 128./127.;
    mat3 TBN = cotangent_frame(N, -V, texcoord);
    return normalize(TBN * map);
}


void main(){
    vec4 texColor = texture(tex, Texcoord);
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
            lightVectorWorld = normalize((lights[i].light_position).xyz - vertexWorld);
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

        vec3 viewDir = normalize(camera_pos - vertexWorld); //reflected
        vec3 halfDir = normalize(lightVectorWorld + viewDir); //eyeVector

        normal = perturb_normal(normal, halfDir, Texcoord);

        //diffuse brightness
        float Kd = dot(lightVectorWorld, normal);
        vec4 diffuse = Kd * diffuse_product;

        //spec brightness
        //blinn
        float specAngle = max(dot(halfDir, normal), 0.0);
        float Ks = pow(specAngle, shininess);
        /*
        //phong
        vec3 reflected = reflect(-lightVectorWorld, normal);
        vec3 eyeVectorWorld = normalize(camera_pos - vertexWorld);
        float s = dot(reflected, eyeVectorWorld);
        //specular brightness
        float Ks = pow(s, shininess);*/
        vec4 specular = Ks * specular_product;

        // Compute terms in the illumination equation
        vec4 ambient = ambient_product;

        lcolor += ambient.xyz + shadowFactor * (attenuation * (clamp(diffuse.xyz, 0, 1) + clamp(specular.xyz, 0, 1)));
    }

  fColor = vec4(Color, 1.0) * texColor * vec4(lcolor, 1.0);
}