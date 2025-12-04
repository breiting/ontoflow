#version 330 core

layout(location = 0) in vec3 iPos;

uniform mat4 u_ViewModel;
uniform mat4 u_Proj;
uniform float u_Radius;

out vec2 vUV;

vec2 getCorner(int id) {
    if (id == 0) return vec2(-1, -1);
    if (id == 1) return vec2( 1, -1);
    if (id == 2) return vec2(-1,  1);
    return vec2( 1,  1);
}

void main() {
    vUV = getCorner(gl_VertexID);
    vec4 centerVS = u_ViewModel * vec4(iPos, 1.0);
    vec2 offset = vUV * u_Radius;
    vec4 posVS = centerVS + vec4(offset.x, offset.y, 0.0, 0.0);
    gl_Position = u_Proj * posVS;
}
