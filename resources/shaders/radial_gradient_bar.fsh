#ifdef GL_ES
precision mediump float;
#endif

// The progress bar's fill: its gradient, or its own colour when it has none,
// with adjustable brightness and saturation for the glow effects.

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D u_texture;

uniform int stopAt;
uniform vec4 colors[24];

uniform vec2 positions[24];


uniform vec2 uvMin;
uniform vec2 uvMax;

// 1: no gradient, keep the sprite's own colour.
uniform int u_tint;

// Multiples of the bar's own; 1 leaves it as it is. Above 1 brightens or
// saturates it.
uniform float u_brightness;
uniform float u_saturation;

vec4 gradientColor(vec2 uv) {
    vec4 finalColor = vec4(0.0);
    float totalWeight = 0.0;

    for (int i = 0; i < 24; ++i) {
        if (i >= stopAt) break;

        float dist = distance(uv, positions[i]);
        dist *= dist;

        float weight = 1.0 / max(dist, 1e-5);

        finalColor += colors[i] * weight;
        totalWeight += weight;
    }

    return finalColor / max(totalWeight, 1e-5);
}


void main() {
    vec4 texColor = texture2D(u_texture, v_texCoord);
    vec4 color;

    if (u_tint == 1)
        color = texColor * v_fragmentColor;
    else
        color = texColor * gradientColor((v_texCoord - uvMin) / (uvMax - uvMin));

    // Opacity always applies, so the bar's second colour can fade over it.
    color.a *= u_tint == 1 ? 1.0 : v_fragmentColor.a;

    float luma = dot(color.rgb, vec3(0.299, 0.587, 0.114));

    color.rgb = mix(vec3(luma), color.rgb, u_saturation) * u_brightness;

    gl_FragColor = color;
}
