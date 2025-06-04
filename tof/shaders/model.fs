#version 330 core
out vec4 FragColor;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 color;
};

in vec3 Position;  
in vec3 Normal;

uniform vec3 color;
uniform vec3 viewPos;
uniform Light light;
uniform float shininess;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{
    // ambient
    vec3 ambient = light.ambient * color;

    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - Position);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * color * light.color;  

    // specular
    vec3 viewDir = normalize(viewPos - Position);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 R = reflect(-viewDir, normalize(Normal));
    vec3 reflection = texture(skybox, R).rgb;
    vec3 specular = light.specular * spec * light.color + 0.05 * reflection;  

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}