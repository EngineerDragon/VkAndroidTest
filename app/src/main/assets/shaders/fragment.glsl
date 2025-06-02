#version 320 es
precision highp float;

in vec2 loc;
out vec4 color;

uniform float tilt;
uniform float orient;
uniform float pressure;
uniform vec2 location;

void main() {
    vec2 relpos = loc-location;
    float cosor = cos(orient);
    float sinor = sin(-orient);
    mat2 rot;
    rot[0] = vec2(cosor , sinor);
    rot[1] = vec2(-sinor , cosor);
    vec2 newvec = vec2(rot * vec2(0.0,-1.0));

    vec3 center = vec3(exp(0.01 - length(relpos) * 20.0f));
    vec3 oricolor = vec3(exp(0.01 - length(relpos - newvec * sin(tilt)) * 20.0f));

    color = vec4(center + oricolor,1.0);
}
