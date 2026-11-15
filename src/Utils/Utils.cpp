#include "Utils.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include "Cache.hpp"

#include "Hooks/SimplePlayer.hpp"

bool GradientConfig::isEmpty(ColorType colorType, bool secondPlayer) {
    if (points.empty()) return true;
    
    ccColor3B color = Utils::getPlayerColor(colorType, secondPlayer);
    
    for (const SimplePoint& point : points)
        if (point.color != color)
            return false;
    
    return true;
}

SimplePlayer* Utils::createIcon(IconType type, bool secondPlayer) {
    SimplePlayer* icon = SimplePlayer::create(1);

    icon->updatePlayerFrame(getIconID(type, secondPlayer), type);
    icon->disableGlowOutline();

    return icon;
}

CCMenuItemToggler* Utils::createTypeToggle(bool radial, CCPoint pos, CCObject* target, SEL_MenuHandler callback) {
    CCSprite* spr = CCSprite::create("GJ_button_04.png");

    CCSprite* spr2 = CCSprite::createWithSpriteFrameName(radial ? "edit_areaModeBtn04_001.png" : "edit_areaModeBtn03_001.png");
    spr2->setScale(1.6f);
    spr2->setPosition(spr->getContentSize() / 2.f);

    spr->addChild(spr2);

    CCSprite* spr3 = CCSprite::create("GJ_button_02.png");

    CCSprite* spr4 = CCSprite::createWithSpriteFrameName(radial ? "edit_areaModeBtn04_001.png" : "edit_areaModeBtn03_001.png");
    spr4->setScale(1.6f);
    spr4->setPosition(spr3->getContentSize() / 2.f);

    spr3->addChild(spr4);

    CCMenuItemToggler* toggle = CCMenuItemToggler::create(spr, spr3, target, callback);
    toggle->setScale(0.475f);
    toggle->setPosition(pos);

    return toggle;
}

ccColor3B Utils::getPlayerColor(ColorType colorType, bool secondPlayer) {
    GameManager* gm = GameManager::get();

    if (colorType == ColorType::White)
        return ccWHITE;
    else if (colorType == ColorType::Line)
        return ccBLACK;

    Mod* sdiMod = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
    int color;
    
    if (secondPlayer && sdiMod && Utils::isSettingEnabled(P2_SEPARATE)) {
        switch (colorType) {
            case ColorType::Glow:
                color = sdiMod->getSavedValue<int>("colorglow", 14);
                break;
            case ColorType::Secondary:
                color = sdiMod->getSavedValue<int>("color2", 14);
                break;
            case ColorType::Main:
            default:
                color = sdiMod->getSavedValue<int>("color1", 13);
        }
        
        return gm->colorForIdx(color);
    }

    if (secondPlayer) {
        if (colorType == ColorType::Main)
            colorType = ColorType::Secondary;
        else if (colorType == ColorType::Secondary)
            colorType = ColorType::Main;
    }
    
    switch (colorType) {
        case ColorType::Glow:
            color = gm->getPlayerGlowColor();
            break;
        case ColorType::Secondary:
            color = gm->getPlayerColor2();
            break;
        case ColorType::Main:
        default:
            color = gm->getPlayerColor();
    }
    
    return gm->colorForIdx(color);
}

int Utils::getIconID(IconType type, bool secondPlayer) {
    GameManager* gm = GameManager::get();

    if (!(secondPlayer && Loader::get()->isModLoaded("weebify.separate_dual_icons")))
        switch (type) {
            case IconType::Cube: return gm->getPlayerFrame();
            case IconType::Ship: return gm->getPlayerShip();
            case IconType::Ball: return gm->getPlayerBall();
            case IconType::Ufo: return gm->getPlayerBird();
            case IconType::Wave: return gm->getPlayerDart();
            case IconType::Robot: return gm->getPlayerRobot();
            case IconType::Spider: return gm->getPlayerSpider();
            case IconType::Swing: return gm->getPlayerSwing();
            case IconType::Jetpack: return gm->getPlayerJetpack();
            default: return gm->getPlayerFrame();
        }
    else {
        Mod* sdiMod =  Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        switch (type) {
            case IconType::Cube: return sdiMod->getSavedValue<int>("cube", 1);
            case IconType::Ship: return sdiMod->getSavedValue<int>("ship", 1);
            case IconType::Ball: return sdiMod->getSavedValue<int>("roll", 1);
            case IconType::Ufo: return sdiMod->getSavedValue<int>("bird", 1);
            case IconType::Wave: return sdiMod->getSavedValue<int>("dart", 1);
            case IconType::Robot: return sdiMod->getSavedValue<int>("robot", 1);
            case IconType::Spider: return sdiMod->getSavedValue<int>("spider", 1);
            case IconType::Swing: return sdiMod->getSavedValue<int>("swing", 1);
            case IconType::Jetpack: return sdiMod->getSavedValue<int>("jetpack", 1);
            default: return sdiMod->getSavedValue<int>("cube", 1);
        }
    }
}

bool Utils::isGradientSaved(GradientConfig config) {
    for (const matjson::Value& obj : Mod::get()->getSavedValue<matjson::Value>("saved-gradients"))
        if (configFromObject(obj) == config)
            return true;

    return false;
}

bool Utils::isSettingEnabled(int setting) {
    switch (setting) {
        case MOD_DISABLED: return Cache::isModDisabled();
        case P2_DISABLED: return Cache::is2PDisabled();
        case P2_FLIP: return Cache::is2PFlip();
        case MENU_GRADIENTS: return Cache::isMenuGradientsEnabled();
        case P2_SEPARATE: return Cache::is2PSeparate();
    }

    return false;
}

GradientConfig Utils::getDefaultConfig(ColorType colorType, bool secondPlayer) {
    ccColor3B color = getPlayerColor(colorType, secondPlayer);

    return GradientConfig{
        {
            {{0.5f, 1.1f}, color},
            {{0.5f, -0.1f}, color}
        },
        true
    };
}

// The progress bar is wide and thin, so its default points sit on its two ends.
GradientConfig Utils::getDefaultBarConfig() {
    ccColor3B color = getPlayerColor(ColorType::Main, false);

    return GradientConfig{
        {
            {{0.f, 0.5f}, color},
            {{1.f, 0.5f}, color}
        },
        true
    };
}

GradientConfig Utils::getSavedBarConfig() {
    if (!Mod::get()->hasSavedValue("progress-bar"))
        return getDefaultBarConfig();

    matjson::Value container = Mod::get()->getSavedValue<matjson::Value>("progress-bar");

    if (!container.contains("bar"))
        return getDefaultBarConfig();

    return configFromObject(container["bar"]);
}

void Utils::saveBarConfig(GradientConfig config) {
    saveConfig(config, "progress-bar", "bar");
}

// CCDrawNode::ensureCapacity is private, but it is the only safe way to grow
// the node's vertex buffer. This grants access to it without changing the
// class.
template <auto Member>
struct DrawNodeAccess {
    friend void ensureDrawNodeCapacity(CCDrawNode* node, unsigned int count) {
        (node->*Member)(count);
    }
};

template struct DrawNodeAccess<&CCDrawNode::ensureCapacity>;

void ensureDrawNodeCapacity(CCDrawNode*, unsigned int);

// Replaces everything a draw node draws with the given triangles.
void Utils::setDrawNodeVertices(CCDrawNode* node, const std::vector<ccV2F_C4B_T2F>& vertices) {
    node->m_nBufferCount = 0;

    if (!vertices.empty()) {
        ensureDrawNodeCapacity(node, static_cast<unsigned int>(vertices.size()));

        std::memcpy(node->m_pBuffer, vertices.data(), vertices.size() * sizeof(ccV2F_C4B_T2F));

        node->m_nBufferCount = static_cast<GLsizei>(vertices.size());
    }

    node->m_bDirty = true;
}

// Player 2 only has its own wave trail when it has its own gradients.
std::string Utils::getWaveID(bool secondPlayer) {
    return secondPlayer && isSettingEnabled(P2_SEPARATE) ? "wave-trail-p2" : "wave-trail";
}

// Wave trail points all sit on one line across the trail: y 0.5 is the middle
// of the trail, y 0 and 1 are its edges. Only the distance from the middle
// matters, so the trail is always symmetric.
GradientConfig Utils::getDefaultWaveConfig(bool secondPlayer) {
    ccColor3B color = getPlayerColor(ColorType::Main, secondPlayer && isSettingEnabled(P2_SEPARATE));

    return GradientConfig{
        {
            {{0.5f, 0.5f}, color},
            {{0.5f, 1.f}, color}
        },
        false
    };
}

GradientConfig Utils::getSavedWaveConfig(bool secondPlayer) {
    std::string id = getWaveID(secondPlayer);

    if (!Mod::get()->hasSavedValue(id))
        return getDefaultWaveConfig(secondPlayer);

    matjson::Value container = Mod::get()->getSavedValue<matjson::Value>(id);

    if (!container.contains("trail"))
        return getDefaultWaveConfig(secondPlayer);

    return configFromObject(container["trail"]);
}

static bool s_waveStyleDirty[2] = { true, true };

void Utils::saveWaveConfig(GradientConfig config, bool secondPlayer) {
    // Pasted or loaded gradients can come from an icon; keep them on the line.
    for (SimplePoint& point : config.points)
        point.pos.x = 0.5f;

    saveConfig(config, getWaveID(secondPlayer), "trail");
    invalidateWaveBands();
}

WaveGlowSettings Utils::getWaveGlow(bool secondPlayer) {
    WaveGlowSettings settings;

    std::string id = getWaveID(secondPlayer);

    if (!Mod::get()->hasSavedValue(id))
        return settings;

    matjson::Value container = Mod::get()->getSavedValue<matjson::Value>(id);

    if (!container.contains("glow"))
        return settings;

    const matjson::Value& glow = container["glow"];

    settings.enabled = glow["enabled"].asBool().unwrapOr(false);
    settings.gain = std::clamp(static_cast<float>(glow["gain"].asDouble().unwrapOr(1.0)), 0.f, 3.f);
    settings.size = std::clamp(static_cast<float>(glow["size"].asDouble().unwrapOr(0.35)), 0.f, 1.f);

    return settings;
}

void Utils::saveWaveGlow(WaveGlowSettings settings, bool secondPlayer) {
    std::string id = getWaveID(secondPlayer);

    matjson::Value container = Mod::get()->getSavedValue<matjson::Value>(id);
    matjson::Value glow = matjson::Value{};

    glow["enabled"] = settings.enabled;
    glow["gain"] = settings.gain;
    glow["size"] = settings.size;

    container["glow"] = glow;

    Mod::get()->setSavedValue(id, container);
    invalidateWaveBands();
}

void Utils::invalidateWaveBands() {
    s_waveStyleDirty[0] = true;
    s_waveStyleDirty[1] = true;
}

// Turns the points into layers ordered from the full-width outer layer to the
// thinnest inner one, so drawing them in order paints the trail's
// cross-section. isLinear means a smooth blend, otherwise each colour is a
// solid band reaching halfway to its neighbours.
std::vector<WaveBand> Utils::computeWaveBands(const GradientConfig& config) {
    std::vector<std::pair<float, ccColor3B>> stops;

    for (const SimplePoint& point : config.points)
        stops.push_back({
            std::clamp(std::abs(point.pos.y - 0.5f) * 2.f, 0.f, 1.f),
            point.color
        });

    std::stable_sort(stops.begin(), stops.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });

    std::vector<WaveBand> bands;

    if (stops.empty()) return bands;

    if (!config.isLinear) {
        bands.push_back({ 1.f, stops.back().second });

        for (int i = static_cast<int>(stops.size()) - 2; i >= 0; i--) {
            float scale = (stops[i].first + stops[i + 1].first) / 2.f;

            if (scale < 0.02f || scale >= bands.back().scale) continue;

            bands.push_back({ scale, stops[i].second });
        }

        return bands;
    }

    const int steps = 16;

    for (int i = 0; i < steps; i++) {
        float scale = 1.f - static_cast<float>(i) / steps;
        float t = scale - 0.5f / steps;

        ccColor3B color = stops.back().second;

        if (t <= stops.front().first)
            color = stops.front().second;
        else
            for (size_t j = 0; j + 1 < stops.size(); j++) {
                float a = stops[j].first;
                float b = stops[j + 1].first;

                if (t > b) continue;

                float local = b - a > 0.0001f ? (t - a) / (b - a) : 1.f;

                color = ccc3(
                    static_cast<GLubyte>(stops[j].second.r + (stops[j + 1].second.r - stops[j].second.r) * local),
                    static_cast<GLubyte>(stops[j].second.g + (stops[j + 1].second.g - stops[j].second.g) * local),
                    static_cast<GLubyte>(stops[j].second.b + (stops[j + 1].second.b - stops[j].second.b) * local)
                );

                break;
            }

        bands.push_back({ scale, color });
    }

    return bands;
}

// Blur radius of the glow, in trail half-widths.
float Utils::getWaveGlowSigma(const WaveGlowSettings& settings) {
    return std::max(0.05f, settings.size * 2.f);
}

// A soft glow is the image blurred and added back on top. Across a straight
// trail that blur only depends on the distance from the middle, so it is
// worked out once here: the trail's cross-section is blurred with a gaussian
// and sampled at a few distances. Drawing those samples as a mesh with
// interpolated colours gives the same look as a real blur for almost no cost.
std::vector<WaveGlowRing> Utils::computeWaveGlow(const std::vector<WaveBand>& bands, const WaveGlowSettings& settings) {
    std::vector<WaveGlowRing> rings;

    if (!settings.enabled || settings.gain <= 0.f || bands.empty()) return rings;

    float sigma = getWaveGlowSigma(settings);
    float reach = 1.f + sigma * 2.5f;

    const int ringCount = 8;
    const int samples = 32;

    for (int i = 0; i < ringCount; i++) {
        float offset = reach * i / (ringCount - 1);
        float sum[3] = { 0.f, 0.f, 0.f };

        // The outermost ring is always dark so the glow fades to nothing.
        for (int j = 0; j < samples && i < ringCount - 1; j++) {
            float x = -1.f + 2.f * (j + 0.5f) / samples;
            float distance = std::abs(x);

            // Bands go from widest to thinnest: the last one that still
            // covers this distance is the one visible there.
            ccColor3B color = bands.front().color;

            for (const WaveBand& band : bands)
                if (band.scale >= distance)
                    color = band.color;

            float weight = std::exp(-(offset - x) * (offset - x) / (2.f * sigma * sigma))
                / (sigma * 2.5066283f) * (2.f / samples);

            sum[0] += color.r * weight;
            sum[1] += color.g * weight;
            sum[2] += color.b * weight;
        }

        rings.push_back({
            offset,
            ccc3(
                static_cast<GLubyte>(std::min(255.f, sum[0] * settings.gain)),
                static_cast<GLubyte>(std::min(255.f, sum[1] * settings.gain)),
                static_cast<GLubyte>(std::min(255.f, sum[2] * settings.gain))
            )
        });
    }

    return rings;
}

// Builds the glow for a trail running through `points` as triangles: strips
// that follow the trail, one per gap between rings on each side, with mitred
// corners so strips of neighbouring segments meet instead of overlapping
// (overlaps would show up as bright spots). Both ends fade out lengthwise.
void Utils::buildWaveGlowMesh(const std::vector<CCPoint>& trail, float halfWidth, const std::vector<WaveGlowRing>& rings, float opacity, std::vector<ccV2F_C4B_T2F>& out) {
    if (trail.size() < 2 || rings.size() < 2 || halfWidth <= 0.f || opacity <= 0.f) return;

    float reach = rings.back().offset * halfWidth;

    std::vector<CCPoint> points;
    std::vector<CCPoint> normals;
    std::vector<float> fades;

    auto direction = [](const CCPoint& from, const CCPoint& to) {
        CCPoint delta = to - from;
        float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);

        return length > 0.0001f ? CCPoint{ delta.x / length, delta.y / length } : CCPoint{ 1.f, 0.f };
    };

    CCPoint firstDir = direction(trail[0], trail[1]);
    CCPoint lastDir = direction(trail[trail.size() - 2], trail.back());

    points.push_back(trail.front() - firstDir * reach);
    normals.push_back({ -firstDir.y, firstDir.x });
    fades.push_back(0.f);

    for (size_t i = 0; i < trail.size(); i++) {
        CCPoint before = i == 0 ? firstDir : direction(trail[i - 1], trail[i]);
        CCPoint after = i + 1 == trail.size() ? lastDir : direction(trail[i], trail[i + 1]);

        CCPoint normalBefore = { -before.y, before.x };
        CCPoint normalAfter = { -after.y, after.x };

        CCPoint miter = normalBefore + normalAfter;
        float length = std::sqrt(miter.x * miter.x + miter.y * miter.y);

        if (length < 0.0001f)
            miter = normalBefore;
        else {
            miter = { miter.x / length, miter.y / length };

            // Stretch so the strip keeps its width through the corner, but
            // not without limit on very sharp turns.
            float cosine = std::max(0.34f, miter.x * normalBefore.x + miter.y * normalBefore.y);

            miter = { miter.x / cosine, miter.y / cosine };
        }

        points.push_back(trail[i]);
        normals.push_back(miter);
        fades.push_back(1.f);
    }

    points.push_back(trail.back() + lastDir * reach);
    normals.push_back({ -lastDir.y, lastDir.x });
    fades.push_back(0.f);

    auto vertex = [&](size_t point, size_t ring, float side) {
        CCPoint pos = points[point] + normals[point] * (rings[ring].offset * halfWidth * side);
        float strength = fades[point] * opacity;

        ccV2F_C4B_T2F ret;

        ret.vertices = { pos.x, pos.y };
        ret.texCoords = { 0.f, 0.f };
        ret.colors = ccc4(
            static_cast<GLubyte>(rings[ring].color.r * strength),
            static_cast<GLubyte>(rings[ring].color.g * strength),
            static_cast<GLubyte>(rings[ring].color.b * strength),
            255
        );

        return ret;
    };

    out.reserve(out.size() + (points.size() - 1) * (rings.size() - 1) * 12);

    for (size_t i = 0; i + 1 < points.size(); i++)
        for (size_t ring = 0; ring + 1 < rings.size(); ring++)
            for (float side : { 1.f, -1.f }) {
                ccV2F_C4B_T2F a = vertex(i, ring, side);
                ccV2F_C4B_T2F b = vertex(i, ring + 1, side);
                ccV2F_C4B_T2F c = vertex(i + 1, ring + 1, side);
                ccV2F_C4B_T2F d = vertex(i + 1, ring, side);

                out.push_back(a);
                out.push_back(b);
                out.push_back(c);

                out.push_back(a);
                out.push_back(c);
                out.push_back(d);
            }
}

// Bands and glow for the in-game trail. Empty bands mean the vanilla trail is
// left alone.
const WaveStyle& Utils::getWaveStyle(bool secondPlayer) {
    static WaveStyle styles[2];

    int index = secondPlayer ? 1 : 0;

    if (s_waveStyleDirty[index]) {
        s_waveStyleDirty[index] = false;

        GradientConfig config = getSavedWaveConfig(secondPlayer);
        std::vector<WaveBand> bands = computeWaveBands(config);

        bool usesPlayerColor = config.isEmpty(
            ColorType::Main,
            secondPlayer && isSettingEnabled(P2_SEPARATE)
        );

        styles[index].glow = computeWaveGlow(bands, getWaveGlow(secondPlayer));
        styles[index].bands = usesPlayerColor ? std::vector<WaveBand>{} : bands;
    }

    return styles[index];
}

// rectInPixels, when given, is the texture area the gradient spans. The
// in-game bar grows by widening its texture rect, so without it the whole
// gradient would be squeezed into the filled part.
void Utils::applyBarGradient(CCSprite* sprite, GradientConfig config, bool inGame, const CCRect* rectInPixels) {
    applyGradient(sprite, config, static_cast<IconType>(-2), ColorType::Main, 800, false, false, inGame, 66, false, rectInPixels);
}

matjson::Value Utils::getSaveObject(GradientConfig config) {
    matjson::Value ret = matjson::Value{};
    matjson::Value pointsObject = matjson::Value::array();

    for (SimplePoint point : config.points) {
        matjson::Value object = matjson::Value{};

        object["pos"]["x"] = point.pos.x;
        object["pos"]["y"] = point.pos.y;

        object["color"]["r"] = point.color.r;
        object["color"]["g"] = point.color.g;
        object["color"]["b"] = point.color.b;

        pointsObject.push(object);
    }

    ret["points"] = pointsObject;
    ret["linear"] = config.isLinear;

    return ret;
}

void Utils::removeSavedGradient(GradientConfig config) {
    matjson::Value newArray = matjson::Value::array();

    for (const matjson::Value& obj : Mod::get()->getSavedValue<matjson::Value>("saved-gradients"))
        if (configFromObject(obj) != config)
            newArray.push(obj);
    
    Mod::get()->setSavedValue("saved-gradients", newArray);
}

void Utils::saveConfig(GradientConfig config, const std::string& id, const std::string& secondId) {
    matjson::Value container = Mod::get()->getSavedValue<matjson::Value>(id);

    if (secondId.empty()) {
        if (!container.isArray()) 
            container = matjson::Value::array();

        container.push(getSaveObject(config));
    } else
        container[secondId] = getSaveObject(config);

    Mod::get()->setSavedValue(id, container);
}

GradientConfig Utils::configFromObject(const matjson::Value& object) {
    GradientConfig config;

    config.isLinear = object["linear"].asBool().unwrapOr(true);

    for (const matjson::Value& point : object["points"])
        config.points.push_back({
            ccp(
                point["pos"]["x"].asDouble().unwrapOr(0.0), 
                point["pos"]["y"].asDouble().unwrapOr(0.0)
            ),
            ccc3(
                point["color"]["r"].asInt().unwrapOr(0),
                point["color"]["g"].asInt().unwrapOr(0),
                point["color"]["b"].asInt().unwrapOr(0)
            )
        });

    return config;
}

GradientConfig Utils::getSavedConfig(IconType type, ColorType colorType, bool secondPlayer) {
    std::string id = getTypeID(type);
    std::string color = "color" + std::to_string(colorType);

    if (!Utils::isSettingEnabled(P2_SEPARATE))
        secondPlayer = false;

    if (secondPlayer)
        id += "-p2";

    GradientConfig config;

    if (!Mod::get()->hasSavedValue(id)) {
        std::string globalKey = secondPlayer ? "global-p2" : "global";

        if (!Mod::get()->hasSavedValue(globalKey)) {
            return getDefaultConfig(colorType, secondPlayer);
        } else
            id = globalKey;
    }

    matjson::Value jsonConfig = Mod::get()->getSavedValue<matjson::Value>(id);

    if (!jsonConfig.contains(color)) {
        return getDefaultConfig(colorType, secondPlayer);
    }

    config = configFromObject(jsonConfig[color]);

    return config;
}

Gradient Utils::getGradient(IconType type, bool secondPlayer) {
    Gradient gradient = {
        getSavedConfig(type, ColorType::Main, secondPlayer),
        getSavedConfig(type, ColorType::Secondary, secondPlayer),
        getSavedConfig(type, ColorType::Glow, secondPlayer),
        getSavedConfig(type, ColorType::White, secondPlayer),
        getSavedConfig(type, ColorType::Line, secondPlayer)
    };

    if (
        secondPlayer
        && Utils::isSettingEnabled(P2_FLIP)
        && !Utils::isSettingEnabled(P2_SEPARATE)
    ) {
        GradientConfig tempConfig = gradient.main;
        gradient.main = gradient.secondary;
        gradient.secondary = tempConfig;
    }

    return gradient;
}

void Utils::setIconColors(SimplePlayer* icon, ColorType colorType, bool white, bool secondPlayer) {
    GameManager* gm = GameManager::get();
    Mod* sdiMod = Loader::get()->getLoadedMod("weebify.separate_dual_icons");

    ccColor3B color1 = white ? ccc3(255, 255, 255)
        : getPlayerColor(ColorType::Main, secondPlayer);

    ccColor3B color2 = white ? ccc3(255, 255, 255)
        : getPlayerColor(ColorType::Secondary, secondPlayer);
    
    bool hasGlowOutline = gm->getPlayerGlow();
    
    if (secondPlayer && sdiMod && Utils::isSettingEnabled(P2_SEPARATE))
        hasGlowOutline = sdiMod->getSavedValue<bool>("glow", false);
            
    ccColor3B colorGlow = white ? ccc3(255, 255, 255)
        : getPlayerColor(ColorType::Glow, secondPlayer);

    icon->setColor(color1);
    icon->setSecondColor(color2);

    icon->m_hasGlowOutline = hasGlowOutline;

    if (icon->m_hasGlowOutline)
        icon->enableCustomGlowColor(colorGlow);
    else
        icon->disableCustomGlowColor();

    icon->updateColors();
}

std::string Utils::getTypeID(IconType type) {
    switch (type) {
        case IconType::Cube: return "cube";
        case IconType::Ship: return "ship";
        case IconType::Ball: return "ball";
        case IconType::Ufo: return "ufo";
        case IconType::Wave: return "wave";
        case IconType::Robot: return "robot";
        case IconType::Spider: return "spider";
        case IconType::Swing: return "swing";
        case IconType::Jetpack: return "jetpack";
        default: return "global";
    }
}

std::string Utils::getTypeID(SpriteType type) {
    switch (type) {
        case SpriteType::Icon: return "icon";
        case SpriteType::Vehicle: return "vehicle";
        case SpriteType::Animation: return "animation";
        default: return "icon";
    }
}

IconType Utils::getIconType(SimplePlayer* icon) {
    return static_cast<ProSimplePlayer*>(icon)->m_fields->m_type; 
}

std::vector<GradientConfig> Utils::getSavedGradients() {
    std::vector<GradientConfig> ret;

    for (const matjson::Value obj : Mod::get()->getSavedValue<matjson::Value>("saved-gradients"))
        ret.push_back(configFromObject(obj));

    return ret;
}

void Utils::applyGradient(SimplePlayer* icon, Gradient gradient, bool blend, bool secondPlayer, int extra) {
    applyGradient(icon, gradient.main, ColorType::Main, blend, secondPlayer, extra);
    applyGradient(icon, gradient.secondary, ColorType::Secondary, blend, secondPlayer, extra);
    applyGradient(icon, gradient.glow, ColorType::Glow, blend, secondPlayer, extra);
    applyGradient(icon, gradient.white, ColorType::White, blend, secondPlayer, extra);
    applyGradient(icon, gradient.line, ColorType::Line, blend, secondPlayer, extra);
}
    
void Utils::applyGradient(SimplePlayer* icon, GradientConfig config, ColorType colorType, bool blend, bool secondPlayer, int extra) {
    GJRobotSprite* otherSprite = nullptr;
    IconType iconType = getIconType(icon);

    if (icon->m_robotSprite) if (icon->m_robotSprite->isVisible()) otherSprite = icon->m_robotSprite;
    if (icon->m_spiderSprite) if (icon->m_spiderSprite->isVisible()) otherSprite = icon->m_spiderSprite;

    if (otherSprite) {
        switch (colorType) {
            case ColorType::Main: {
                int id = 100;
                
                for (CCSpritePart* spr : otherSprite->m_headSprite->getParent()->getChildrenExt<CCSpritePart*>()) {
                    if (!typeinfo_cast<CCSpritePart*>(spr)) continue;
                    
                    id++;
                    
                    applyGradient(spr, config, iconType, colorType, id, blend, secondPlayer, false, extra);
                }
                
                break;
            }
            case ColorType::Secondary: {
                int id = 200;
            
                for (CCSprite* spr : CCArrayExt<CCSprite*>(otherSprite->m_secondArray)) {
                    if (!typeinfo_cast<CCSprite*>(spr) || spr == otherSprite->m_headSprite) continue;

                    id++;
                    
                    applyGradient(spr, config, iconType, colorType, id, blend, secondPlayer, false, extra);
                }
                
                break;
            }
            case ColorType::Glow: {
                int id = 300;
                
                for (CCSprite* spr : otherSprite->m_glowSprite->getChildrenExt<CCSprite*>()) {
                    id++;
                    
                    applyGradient(spr, config, iconType, colorType, id, blend, secondPlayer, false, extra);
                }

                break;
            }
            case ColorType::White: {
                applyGradient(otherSprite->m_extraSprite, config, iconType, colorType, 400, blend, secondPlayer, false, extra);
                otherSprite->m_extraSprite->setZOrder(2);
                break;
            }
            case ColorType::Line: {
                int id = 500;
                for (CCSpritePart* spr : otherSprite->m_headSprite->getParent()->getChildrenExt<CCSpritePart*>()) {
                    if (!typeinfo_cast<CCSpritePart*>(spr)) continue;

                    id++;

                    CCSprite* lineSprite;
                    if ((lineSprite = typeinfo_cast<CCSprite*>(spr->getChildByID("gradient-line"_spr)))) {
                        lineSprite->setDisplayFrame(spr->displayFrame());
                    } else {
                        lineSprite = CCSprite::createWithSpriteFrame(spr->displayFrame());
                        lineSprite->setID("gradient-line"_spr);

                        spr->addChild(lineSprite, 1);
                    }

                    lineSprite->setContentSize(spr->getContentSize());
                    lineSprite->setPosition(spr->getContentSize() / 2.f);

                    lineSprite->setVisible(!config.isEmpty(ColorType::Line, secondPlayer));

                    applyGradient(lineSprite, config, iconType, colorType, id, blend, secondPlayer, false, extra, true);
                }
                id = 600;
                for (CCSprite* spr : CCArrayExt<CCSprite*>(otherSprite->m_secondArray)) {
                    if (!typeinfo_cast<CCSprite*>(spr) || spr == otherSprite->m_headSprite) continue;

                    id++;

                    CCSprite* lineSprite;
                    if ((lineSprite = typeinfo_cast<CCSprite*>(spr->getChildByID("gradient-line2"_spr)))) {
                        lineSprite->setDisplayFrame(spr->displayFrame());
                    } else {
                        lineSprite = CCSprite::createWithSpriteFrame(spr->displayFrame());
                        lineSprite->setID("gradient-line2"_spr);

                        spr->addChild(lineSprite, 1);
                    }

                    lineSprite->setContentSize(spr->getContentSize());
                    lineSprite->setPosition(spr->getContentSize()/2);

                    lineSprite->setVisible(!config.isEmpty(ColorType::Line, secondPlayer));

                    applyGradient(lineSprite, config, iconType, colorType, id, blend, secondPlayer, false, extra, true);
                }
                id = 700;
                CCSprite* lineSprite;
                if ((lineSprite = typeinfo_cast<CCSprite*>(otherSprite->m_extraSprite->getChildByID("gradient-line"_spr)))) {
                    lineSprite->setDisplayFrame(otherSprite->m_extraSprite->displayFrame());
                } else {
                    lineSprite = CCSprite::createWithSpriteFrame(otherSprite->m_extraSprite->displayFrame());
                    lineSprite->setID("gradient-line"_spr);

                    otherSprite->m_extraSprite->addChild(lineSprite, 1);
                }

                lineSprite->setContentSize(otherSprite->m_extraSprite->getContentSize());
                lineSprite->setPosition(otherSprite->m_extraSprite->getContentSize() / 2.f);

                lineSprite->setVisible(!config.isEmpty(ColorType::Line, secondPlayer));

                applyGradient(lineSprite, config, iconType, colorType, id, blend, secondPlayer, false, extra, true);

                break;
            }
        }
        
        return;
    }
    
    CCSprite* sprite = nullptr;
    CCSprite* sprite2 = nullptr;
    CCSprite* sprite3 = nullptr;
    int id = 0;
    int id2 = 0;
    int id3 = 0;

    switch (colorType) {
        case ColorType::Main:
            id = 100;
            sprite = icon->m_firstLayer;
            break;
        case ColorType::Secondary:
            id = 200;
            sprite = icon->m_secondLayer;
            break;
        case ColorType::Glow:
            id = 300;
            sprite = icon->m_outlineSprite;
            break;
        case ColorType::White:
            id = 400;
            sprite = icon->m_detailSprite;
            sprite->setZOrder(2);
            break;
        case ColorType::Line: {
            id = 500;
            id2 = 600;
            id3 = 700;
            if (CCSprite* lineSprite = typeinfo_cast<CCSprite*>(icon->m_firstLayer->getChildByID("gradient-line"_spr))) {
                lineSprite->setDisplayFrame(icon->m_firstLayer->displayFrame());
                sprite = lineSprite;
            } else {
                sprite = CCSprite::createWithSpriteFrame(icon->m_firstLayer->displayFrame());
                sprite->setID("gradient-line"_spr);

                icon->m_firstLayer->addChild(sprite, 1);
                
                if (iconType == IconType::Ball && Loader::get()->isModLoaded("alphalaneous.fine_outline"))
                    sprite->runAction(CCRepeatForever::create(
                        CCSequence::create(
                            CCShow::create(),
                            nullptr
                        )
                    ));
            }
            if (CCSprite* lineSprite = typeinfo_cast<CCSprite*>(icon->m_secondLayer->getChildByID("gradient-line"_spr))) {
                lineSprite->setDisplayFrame(icon->m_secondLayer->displayFrame());
                sprite2 = lineSprite;
            } else {
                sprite2 = CCSprite::createWithSpriteFrame(icon->m_secondLayer->displayFrame());
                sprite2->setID("gradient-line"_spr);

                icon->m_secondLayer->addChild(sprite2, 1);
                
                if (iconType == IconType::Ball && Loader::get()->isModLoaded("alphalaneous.fine_outline"))
                    sprite2->runAction(CCRepeatForever::create(
                        CCSequence::create(
                            CCShow::create(),
                            nullptr
                        )
                    ));
            }
            if (CCSprite* lineSprite = typeinfo_cast<CCSprite*>(icon->m_detailSprite->getChildByID("gradient-line"_spr))) {
                lineSprite->setDisplayFrame(icon->m_detailSprite->displayFrame());
                sprite3 = lineSprite;
            } else {
                sprite3 = CCSprite::createWithSpriteFrame(icon->m_detailSprite->displayFrame());
                sprite3->setID("gradient-line"_spr);
                
                if (iconType == IconType::Ball && Loader::get()->isModLoaded("alphalaneous.fine_outline"))
                    sprite3->runAction(CCRepeatForever::create(
                        CCSequence::create(
                            CCShow::create(),
                            nullptr
                        )
                    ));

                icon->m_detailSprite->addChild(sprite3, 1);
            }

            sprite->setContentSize(icon->m_firstLayer->getContentSize());
            sprite2->setContentSize(icon->m_secondLayer->getContentSize());
            sprite3->setContentSize(icon->m_detailSprite->getContentSize());
            sprite->setPosition(icon->m_firstLayer->getContentSize()/2);
            sprite2->setPosition(icon->m_secondLayer->getContentSize()/2);
            sprite3->setPosition(icon->m_detailSprite->getContentSize() / 2.f);

            sprite->setVisible(!config.isEmpty(ColorType::Line, secondPlayer));
            sprite2->setVisible(!config.isEmpty(ColorType::Line, secondPlayer));
            sprite3->setVisible(!config.isEmpty(ColorType::Line, secondPlayer));

            break;
        }
    }

    applyGradient(sprite, config, iconType, colorType, id, blend, secondPlayer, false, extra, colorType == ColorType::Line);
    
    if (sprite2)
        applyGradient(sprite2, config, iconType, colorType, id2, blend, secondPlayer, false, extra, colorType == ColorType::Line);
    
    if (sprite3)
        applyGradient(sprite3, config, iconType, colorType, id3, blend, secondPlayer, false, extra, colorType == ColorType::Line);
}

CCGLProgram* Utils::createShader(const std::string& key, bool linear, bool blend, bool line) {
    CCShaderCache* cache = CCShaderCache::sharedShaderCache();
    CCGLProgram* program = cache->programForKey(key.c_str());
     
    if (!program || key.empty()) {
        static std::filesystem::path vertPath = Mod::get()->getResourcesDir() / "position.vert";
        std::filesystem::path shaderPath = Mod::get()->getResourcesDir() / fmt::format("{}_gradient{}.fsh", linear ? "linear" : "radial", line ? "_line" : blend ? "_blend" : "");

        if (!std::filesystem::exists(vertPath) || !std::filesystem::exists(shaderPath))
            return nullptr;

        program = new CCGLProgram();
        
        if (!key.empty())
            program->retain();
        else    
            program->autorelease();

        program->initWithVertexShaderFilename(
            string::pathToString(vertPath).c_str(),
            string::pathToString(shaderPath).c_str()
        );

        program->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
        program->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
        program->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);

        program->link();
        program->updateUniforms();

        if (!key.empty())
            cache->addProgram(program, key.c_str());
    }
    
    return program;
}

void Utils::applyGradient(CCSprite* sprite, GradientConfig config, IconType iconType, ColorType colorType, int id, bool blend, bool secondPlayer, bool playerObject, int extra, bool line, const CCRect* rectOverride) {
    if (!sprite) return;
    
    if (config.isEmpty(colorType, secondPlayer))
        return sprite->setShaderProgram(
            CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor)
        );
    
    CCGLProgram* program = nullptr;
    
    if (extra != -4732) {
        std::string key = fmt::format(
            "{}-{}-{}-{}-{}-{}-{}-{}"_spr,
            config.isLinear,
            static_cast<int>(iconType),
            id,
            blend,
            line,
            secondPlayer,
            playerObject,
            extra
        );
        
        program = createShader(key, config.isLinear, blend, line);
    } else {
        program = createShader("", config.isLinear, blend, line);
    }
    
    if (!program) return;
    
    sprite->setShaderProgram(program);
    // sprite->setColor({255, 255, 255}); // will this break everything? lets find out // it broke everything

    program->use();
    program->setUniformsForBuiltins();

    CCSpriteFrame* frame = sprite->displayFrame();
    CCRect rectInPixels = rectOverride ? *rectOverride : frame->getRectInPixels();
    CCSize texSize = frame->getTexture()->getContentSizeInPixels();

    bool rot = rectOverride ? false : frame->m_bRotated;

    float uMin = rectInPixels.origin.x / texSize.width;
    float vMin = rectInPixels.origin.y / texSize.height;
    float uMax = (rectInPixels.origin.x + rectInPixels.size.width) / texSize.width;
    float vMax = (rectInPixels.origin.y + rectInPixels.size.height) / texSize.height;

    GLint locMin = glGetUniformLocation(program->getProgram(), "uvMin");
    GLint locMax = glGetUniformLocation(program->getProgram(), "uvMax");
    glUniform2f(locMin, uMin, vMin);
    glUniform2f(locMax, uMax, vMax);
    
    if (colorType == ColorType::Line) {
        GLint locPixelSize = glGetUniformLocation(program->getProgram(), "u_pixelSize");
        glUniform2f(locPixelSize, 1.f / texSize.width, 1.f / texSize.height);
        
        float threshold = 1.f;
        
        if (!Cache::get().m_increaseLineTolerance) {
            threshold = -10.f;
            
            switch (GameManager::get()->m_texQuality) {
                case 1: threshold = -1.25f; break;
                case 2: threshold = -2.5f; break;
            };
        
            #ifdef GEODE_IS_MOBILE
        
            if (!Loader::get()->isModLoaded("weebify.high-graphics-android"))
                threshold = -2.5f;
        
            #endif
        }
        
        GLint locThreshold = glGetUniformLocation(program->getProgram(), "u_threshold");
        glUniform1f(locThreshold, threshold);
    }

    std::vector<ccColor4F> colors;
    int stopAt = config.points.size();

    for (const SimplePoint& point : config.points)
        colors.push_back(ccc4FFromccc3B(point.color));

    if (config.isLinear) {
        CCPoint startPoint = ccp(0, 0);
        CCPoint endPoint = ccp(0, 0);

        float distance = 0.f;

        for (const SimplePoint& point : config.points) {
            float currentDistance = ccpDistance(point.pos, {0.5f, 0.5f});
            if (currentDistance > distance) {
                startPoint = point.pos;
                distance = currentDistance;
            }
        }

        distance = 0.f;

        for (const SimplePoint& point : config.points) {
            float currentDistance = ccpDistance(point.pos, startPoint);
            if (currentDistance > distance) {
                endPoint = point.pos;
                distance = currentDistance;
            }
        }

        std::vector<float> stops;
        distance = ccpDistance(startPoint, endPoint);

        for (const SimplePoint& point : config.points)
            stops.push_back(ccpDistance(point.pos, startPoint) / distance);

        for (size_t i = 0; i < stops.size(); ++i)
            for (size_t j = i + 1; j < stops.size(); ++j)
                if (stops[i] > stops[j]) {
                    std::swap(stops[i], stops[j]);
                    std::swap(colors[i], colors[j]);
                }

        GLint startPointLoc = glGetUniformLocation(program->getProgram(), "startPoint");
        GLint endPointLoc   = glGetUniformLocation(program->getProgram(), "endPoint");

        glUniform2f(startPointLoc, rot ? startPoint.y : startPoint.x, rot ? startPoint.x : (1 - startPoint.y));
        glUniform2f(endPointLoc, rot ? endPoint.y : endPoint.x, rot ? endPoint.x : (1 - endPoint.y));

        GLint stopsLoc = glGetUniformLocation(program->getProgram(), "stops");
        glUniform1fv(stopsLoc, stopAt, stops.data());
    } else {
        std::vector<float> positions;

        for (const SimplePoint& point : config.points) {
            if (rot) {
                positions.push_back(point.pos.y);
                positions.push_back(point.pos.x);
            } else {
                positions.push_back(point.pos.x);
                positions.push_back(1 - point.pos.y);
            }
        }

        GLint loc = glGetUniformLocation(program->getProgram(), "positions");
        glUniform2fv(loc, stopAt, positions.data());
    }

    GLint stopAtLoc = glGetUniformLocation(program->getProgram(), "stopAt");
    glUniform1i(stopAtLoc, stopAt);

    std::vector<GLfloat> colorsData;
    for (const ccColor4F& color : colors) {
        colorsData.push_back(color.r);
        colorsData.push_back(color.g);
        colorsData.push_back(color.b);
        colorsData.push_back(color.a);
    }

    GLint colorsLoc = glGetUniformLocation(program->getProgram(), "colors");
    glUniform4fv(colorsLoc, stopAt, colorsData.data());
}

void Utils::patchBatchNode(CCSpriteBatchNode* node) {
    if (!node) return;
    
    static void* vtable = []() -> void* {
        FakeSpriteBatchNode temp;
        return *(void**)&temp;
    }();

    *(void**)node = vtable;
}

void Utils::hideSprite(CCSprite* sprite) {
    CCGLProgram* shader = CCShaderCache::sharedShaderCache()->programForKey("invis-shader"_spr);
    
    if (shader)
        return sprite->setShaderProgram(shader);
    
    shader = new CCGLProgram();
    shader->initWithVertexShaderByteArray(
        R"(
            attribute vec4 a_position;
            attribute vec2 a_texCoord;
            attribute vec4 a_color;
            
            #ifdef GL_ES
            varying lowp vec4 v_fragmentColor;
            varying mediump vec2 v_texCoord;
            #else
            varying vec4 v_fragmentColor;
            varying vec2 v_texCoord;
            #endif
            
            void main()
            {
                gl_Position = CC_MVPMatrix * a_position;
                v_fragmentColor = a_color;
                v_texCoord = a_texCoord;
            }
        )",
        R"(
            #ifdef GL_ES
            precision mediump float;
            #endif
    
            void main() {
                gl_FragColor = vec4(0.0);
            }
        )"
    );
    shader->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
    shader->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
    shader->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
    shader->link();
    shader->updateUniforms();
    shader->retain();
    
    CCShaderCache::sharedShaderCache()->addProgram(shader, "invis-shader"_spr);
    
    sprite->setShaderProgram(shader);
}