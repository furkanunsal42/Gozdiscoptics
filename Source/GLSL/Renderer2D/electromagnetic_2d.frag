#<fragment shader>
#version 460 core 

layout (location = 0) out vec4 frag_color;

in vec3 v_position;
in vec2 v_texcoord;

layout(binding = 0) uniform sampler3D electric_texture;
layout(binding = 1) uniform sampler3D magnetic_texture;
layout(binding = 2) uniform sampler3D property_texture;

uniform vec3 texture_resolution;
uniform int render_depth;

void main(){
    float   electric = texture(electric_texture, vec3(v_texcoord, render_depth / texture_resolution.z + 0.5 / texture_resolution.z)).x;
    vec2    magnetic = texture(magnetic_texture, vec3(v_texcoord, render_depth / texture_resolution.z + 0.5 / texture_resolution.z)).xy;
    vec4    property = texture(property_texture, vec3(v_texcoord, render_depth / texture_resolution.z + 0.5 / texture_resolution.z));

    const vec4 electric_color   = 1.2 * vec4(0.4, 0.69, 1, 1);
    const vec4 magnetic_color   = 1.2 * vec4(0.94, 0.49, 0.18, 1);
    const vec4 border_color     = 1.2 * vec4(0.12, 0.12, 0.12, 1);

    //if (property.x == 1)
    //    frag_color = border_color;
    //else

    float coeff_electric = log(max(1 + electric, 0.1));

    vec4 color = electric > 0 ? abs(coeff_electric) * electric_color : abs(coeff_electric) * magnetic_color;
    
    if (round(property.x) == 1)
        color = border_color;

    frag_color = vec4(color.xyz, 1);

}