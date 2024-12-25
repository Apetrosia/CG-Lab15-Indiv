#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 

struct Light {
    //vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
  
uniform bool useLightmap;
uniform bool useCircles;

uniform sampler2D texture_normal1; // lightmap
uniform vec3 viewPos;
uniform Material material;
uniform Light light;
uniform vec3 center;
uniform float time;

void main()
{
    float distance = length(FragPos - center);

    vec3 pixColor = vec3(1.0, 0.0, 0.0);

    if (sin((distance * 300.0f + time * 0.3)) < 0.0)
        pixColor = vec3(1.0, 1.0, 1.0);

    vec3 result = pixColor;

    if (useCircles)
    {
        // ambient
        vec3 ambient = light.ambient * pixColor;
        
        // diffuse 
        vec3 norm = normalize(Normal);
        // vec3 lightDir = normalize(light.position - FragPos);
        vec3 lightDir = normalize(-light.direction);  
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * diff * pixColor; 
    
        // specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;  
            
        result = ambient + diffuse + specular;
    }
    else
    {
        // ambient
        vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
    
        if (useLightmap)
            ambient *= texture(texture_normal1, TexCoords).rgb;
        
        // diffuse 
        vec3 norm = normalize(Normal);
        // vec3 lightDir = normalize(light.position - FragPos);
        vec3 lightDir = normalize(-light.direction);  
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb; 
        
        if (useLightmap)
            diffuse *= texture(texture_normal1, TexCoords).rgb;
    
        // specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;  
            
        result = ambient + diffuse + specular;
    }

    FragColor = vec4(result, 1.0);
}
