#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

//components
vec3 ambient;
float ambientStrength = 0.1f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

  

// point light components
vec3 Pposition = vec3(-3.179, 1.49f, -0.34f); 
float Pconstant = 1.0f;
float Plinear = 0.22f;
float Pquadratic = 0.20f;  	

vec3 viewDir;

void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
    
}

vec3 CalcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(Pposition - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), specularStrength);
    // attenuation
    float distance    = length(Pposition - fragPos);
    float attenuation = 1.0 / (Pconstant + Plinear * distance + 
  			     Pquadratic * (distance * distance));    
    // combine results
    vec3 ambient1  = ambientStrength  * vec3(texture(diffuseTexture, fTexCoords));
    vec3 diffuse1  = lightColor  * diff * vec3(texture(diffuseTexture, fTexCoords));
    vec3 specular1 = specularStrength * spec * vec3(texture(specularTexture, fTexCoords));

    ambient1  *= attenuation;
    diffuse1  *= attenuation;
    specular1 *= attenuation;
    return (ambient1 + diffuse1 + specular1);
} 

void main() 
{
    computeDirLight();

    //compute final vertex color
    vec3 color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
    color += CalcPointLight(fNormal, fPosition, viewDir);  

    fColor = vec4(color, 1.0f);
}
