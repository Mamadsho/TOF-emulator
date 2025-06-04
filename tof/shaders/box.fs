#version 330 core
out vec4 FragColor;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 color;
};

in vec3 FragPos;  
in vec3 Normal;
in vec3 TexCoords;

uniform vec3 viewPos;
uniform Light light;

uniform samplerCube skybox;
uniform float shininess;


void main()
{
    // ambient
    vec3 ambient = light.ambient * texture(skybox, TexCoords).rgb;
    // diffuse 
    vec3 norm = -normalize(Normal);

    vec3 lightDir = normalize(light.position - FragPos);
    // vec3 lightDir = normalize(-light.direction);  
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(skybox, TexCoords).rgb * light.color;  

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = light.specular * spec * light.color;  

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}