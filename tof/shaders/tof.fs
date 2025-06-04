#version 330 core
out vec4 FragColor;

in float Tof;

vec3 rainbow(float t) {
    t = clamp(t, 0.0, 1.0);
    
    // Colors gradient
    vec3 colors[6];
    colors[0] = vec3(1.0, 0.0, 0.0); // Red
    colors[1] = vec3(1.0, 0.5, 0.0); // Orange  
    colors[2] = vec3(1.0, 1.0, 0.0); // Yellow
    colors[3] = vec3(0.0, 1.0, 0.0); // Green
    colors[4] = vec3(0.0, 1.0, 1.0); // Cyan
    colors[5] = vec3(0.0, 0.0, 1.0); // Blue
    
    // Scale t to colors count
    float scaledT = t * 5.0;
    int index = int(floor(scaledT));
    float frac = fract(scaledT);
    
    // Interpolate between adjacent colors
    return mix(colors[index], colors[(index + 1) % 6], frac);
}

void main()
{
    FragColor = vec4(rainbow(Tof), 1.0);
}

