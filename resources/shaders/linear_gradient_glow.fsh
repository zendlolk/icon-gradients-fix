#ifdef GL_ES
precision mediump float;
#endif

// Soft glow for a sprite: the sprite's silhouette, blurred, in the colours of
// its gradient. Drawn on a quad bigger than the sprite so the glow has room.

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D u_texture;

uniform int stopAt;
uniform vec4 colors[24];
uniform float stops[24];
uniform vec2 startPoint;
uniform vec2 endPoint;

// Area of the texture the gradient spans.
uniform vec2 uvMin;
uniform vec2 uvMax;

// Area of the texture that belongs to the sprite; anything outside it is
// another sprite on the same sheet and must not be sampled.
uniform vec2 u_maskMin;
uniform vec2 u_maskMax;

// Blur samples: offset (in units of u_radius) and weight.
uniform vec3 u_taps[36];
uniform vec2 u_radius;

uniform float u_gain;
// Scales the whole glow, for glows that change while playing.
uniform float u_strength;
// How much light the glow adds on top of the sprite itself.
uniform float u_inner;

// 0: blur the texture. 1: the shape is a bar filled up to u_fill, which can
// be blurred exactly without sampling.
uniform int u_mode;
uniform float u_fill;
uniform vec2 u_barSize;
uniform float u_sigma;

vec4 gradientColor(vec2 uv) {
    if (stopAt <= 1) return colors[0];

    vec2 dir = endPoint - startPoint;
    float len = length(dir);

    if (len < 1e-6) return colors[0];

    float t = clamp(dot(uv - startPoint, dir / len) / len, 0.0, 1.0);

    if (t <= stops[0]) return colors[0];

    for (int i = 0; i < 23; ++i) {
        if (i >= stopAt - 1) break;

        float a = stops[i];
        float b = stops[i + 1];

        if (t <= b) return mix(colors[i], colors[i + 1], (t - a) / (b - a));
    }

    return colors[stopAt - 1];
}

float alphaAt(vec2 coord) {
    vec2 inside = step(u_maskMin, coord) * step(coord, u_maskMax);

    return texture2D(u_texture, coord).a * inside.x * inside.y;
}

float edge(float x) {
    return smoothstep(-1.6 * u_sigma, 1.6 * u_sigma, x);
}

void main() {
    vec2 uv = (v_texCoord - uvMin) / (uvMax - uvMin);

    float shape = 0.0;
    float blurred = 0.0;
    float lit = 1.0;

    if (u_mode == 1) {
        vec2 pos = uv * u_barSize;
        float width = u_fill * u_barSize.x;

        shape = step(0.0, pos.x) * step(pos.x, width) * step(0.0, pos.y) * step(pos.y, u_barSize.y);
        blurred = edge(pos.x) * edge(width - pos.x) * edge(pos.y) * edge(u_barSize.y - pos.y);

        uv = clamp(uv, vec2(0.0), vec2(u_fill, 1.0));
    } else {
        shape = alphaAt(v_texCoord);

        // How bright the sprite is here, so its dark outlines stay dark.
        lit = dot(texture2D(u_texture, v_texCoord).rgb, vec3(0.299, 0.587, 0.114));

        for (int i = 0; i < 36; ++i)
            blurred += alphaAt(v_texCoord + u_taps[i].xy * u_radius) * u_taps[i].z;

        uv = clamp(uv, vec2(0.0), vec2(1.0));
    }

    // Around the sprite: the blurred shape, levelling off smoothly instead of
    // hitting a hard ceiling when the gain is high.
    float outer = (1.0 - exp(-blurred * u_gain * 1.5)) * (1.0 - shape);

    // On the sprite: an even lift in brightness. It does not follow the blur,
    // which would show up as shapes inside the sprite.
    float inner = u_inner * lit * shape;

    outer *= u_strength;
    inner *= u_strength;

    // Premultiplied: around the sprite this blends over the background, on
    // top of the sprite it only adds light.
    gl_FragColor = vec4(gradientColor(uv).rgb * (outer + inner), outer * 0.85) * v_fragmentColor.a;
}
