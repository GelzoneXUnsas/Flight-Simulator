#version 410 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform sampler2D tex;
uniform sampler2D tex2;

void main()
{
    vec3 lightpos = vec3(100,100,100);
    vec3 lightdir = normalize(lightpos - vertex_pos);
    vec3 camdir = normalize(campos - vertex_pos);
    vec3 frag_norm = normalize(vertex_normal);

    float diffuse_fact = clamp(dot(lightdir,frag_norm),0,1);
    diffuse_fact = 0.3 + diffuse_fact * 0.7;

    vec3 view_half = normalize(camdir + lightdir);
    float spec_fact = clamp(dot(view_half,frag_norm),0,1);
    color.rgb =  (color.rgb*diffuse_fact + vec3(12. / 255., 22. / 255., 28. / 255.)*spec_fact*2) * 20.;
    color.a=0.5;
}
