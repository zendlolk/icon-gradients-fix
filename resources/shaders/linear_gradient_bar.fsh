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

uniform float stops[24];
uniform vec2 startPoint;
uniform vec2 endPoint;


uniform vec2 uvMin;
uniform vec2 uvMax;

// 1: no gradient, keep the sprite's own colour.
uniform int u_tint;

// Multiples of the bar's own; 1 leaves it as it is. Above 1 brightens or
// saturates it.
uniform float u_brightness;
uniform float u_saturation;

// Level-complete animations. 0: none. 1: the gradient repeats along the bar
// and scrolls right, each copy mirrored so it runs into the next without a
// seam. 2: a band of light sweeps across.
uniform int u_anim;
uniform float u_animPhase;
// Copies across the bar for 1, width of the light for 2.
uniform float u_animSize;
// Fades everything to white, for the flash on completion.
uniform float u_white;

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


vec4 animatedColor(vec2 uv) {
    if (u_anim == 1) {
        float t = uv.x * u_animSize - u_animPhase;
        float f = fract(t * 0.5) * 2.0;
        float p = f < 1.0 ? f : 2.0 - f;

        return gradientColor(startPoint + (endPoint - startPoint) * p);
    }

    vec4 color = gradientColor(uv);

    if (u_anim == 2) {
        float centre = fract(u_animPhase) * 1.6 - 0.3;
        float d = (uv.x - centre) / max(u_animSize, 0.001);

        color.rgb = mix(color.rgb, vec3(1.0), exp(-d * d) * 0.65);
    }

    return color;
}

// Death animations. 1: the bar cracks open in the middle of what is
// filled. 2: it shatters into shards that shrink and drift. Both fade out.
uniform int u_deathStyle;
uniform float u_death; // 0 to 1, how far along
uniform float u_deathPoint; // how much of the bar is filled

float hash(float n) {
    return fract(sin(n) * 43758.5453);
}

void main() {
    vec2 uv = (v_texCoord - uvMin) / (uvMax - uvMin);
    vec2 texCoord = v_texCoord;
    float keep = 1.0;

    // Moves where each piece is drawn from, so the pieces move apart.
    if (u_death > 0.0 && u_deathStyle > 0) {
        vec2 span = uvMax - uvMin;

        if (u_deathStyle == 1) {
            float crack = u_deathPoint * 0.5 + (hash(floor(uv.y * 5.0)) - 0.5) * 0.05;
            float side = uv.x < crack ? -1.0 : 1.0;
            float from = uv.x - side * u_death * 0.08;

            keep = side < 0.0 ? step(from, crack) : step(crack, from);
            uv.x = from;
        } else if (u_deathStyle == 2) {
            float columns = 12.0;
            float fill = max(u_deathPoint, 0.01);

            vec2 grid = vec2(uv.x / fill * columns, uv.y * 2.0);
            vec2 cell = floor(grid);
            float n = cell.x * 7.0 + cell.y * 13.0;

            vec2 drift = (vec2(hash(n), hash(n + 1.0)) - 0.5) * 0.6 * u_death;
            vec2 shard = (fract(grid) - 0.5 - drift) / max(1.0 - u_death, 0.001);

            keep = step(abs(shard.x), 0.5) * step(abs(shard.y), 0.5);
            uv = vec2((cell.x + 0.5 + shard.x) / columns * fill, (cell.y + 0.5 + shard.y) / 2.0);
        }

        texCoord = uvMin + uv * span;
        keep *= 1.0 - smoothstep(0.55, 1.0, u_death);
    }

    vec4 texColor = texture2D(u_texture, texCoord);
    vec4 color;

    if (u_tint == 1)
        color = texColor * v_fragmentColor;
    else
        color = texColor * animatedColor(uv);

    // Opacity always applies, so the bar's second colour can fade over it.
    // Sprites are drawn premultiplied, so the colour fades with it; the
    // sprite's own colour already has its opacity in it.
    if (u_tint != 1)
        color *= v_fragmentColor.a;

    float luma = dot(color.rgb, vec3(0.299, 0.587, 0.114));

    color.rgb = mix(vec3(luma), color.rgb, u_saturation) * u_brightness;

    // Premultiplied, so white is as bright as the sprite is opaque.
    color.rgb = mix(color.rgb, vec3(color.a), u_white);

    gl_FragColor = color * keep;
}
