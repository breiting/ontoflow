#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform vec4 u_Color;    // (r,g,b,a)

void main() {
    float d = length(vUV);

    if (d > 1.0)
        discard;

    float inner = 0.4;
    float outer = 1.0;

    if (d < inner)
        discard;

    float alpha = smoothstep(outer, outer - 0.05, d);
    FragColor = vec4(u_Color.rgb, u_Color.a * alpha);
}
