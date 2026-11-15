#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

const int MOD_DISABLED = 1;
const int P2_DISABLED = 2;
const int P2_FLIP = 3;
const int MENU_GRADIENTS = 4;
const int P2_SEPARATE = 5;

enum SpriteType {
    Icon = 1,
    Vehicle = 2,
    Animation = 3
};

enum ColorType {
    Main = 1,
    Secondary = 2,
    Glow = 3,
    White = 4,
    Line = 5,
};

enum class EditorPage {
    Icons = 0,
    Bar = 1,
    Wave = 2
};

// One point of the wave trail's cross-section: the colour at `offset` from the
// middle of the trail, in trail half-widths (so 1 and -1 are its edges, and
// anything beyond is glow). Colours are interpolated between points.
struct WaveNode {
    float offset;
    ccColor4B color;
};

// What a glow belongs to. Icons and the wave trail have one per player.
enum class GlowTarget {
    Icon = 0,
    Bar = 1,
    Wave = 2
};

// One end of a straight piece of wave trail: where its centre line ends, and
// the directions from there to the trail's edge on either side. They are
// separate because at a corner the two sides are cut at different angles.
struct WaveEnd {
    CCPoint point;
    CCPoint positive;
    CCPoint negative;
};

struct GlowSettings {
    bool enabled = false;
    float gain = 0.5f; // 0 to 2
    float size = 0.5f; // 0 to 1
};

// How the progress bar's glow changes while playing.
struct BarGlowEffects {
    // Glow grows from nothing as the bar fills.
    bool buildUp = false;
    bool buildExponential = false;
    float buildPower = 2.f; // only when exponential: higher = later and steeper

    // Glow pulses, faster the closer the bar is to full.
    bool flash = false;
    float flashStart = 0.8f; // progress (0 to 1) where it begins
    float flashSpeed = 0.5f; // 0 to 1
    float flashIntensity = 0.7f; // 0 to 1, how far each flash dims the glow
    bool flashExponential = false;
    float flashPower = 2.f; // only when exponential: how late the pace picks up

    // From a start position: whether to flash at all, and how much of the
    // level has to be played first.
    bool flashFromStartPos = false;
    float startPosOffset = 0.5f;
};

struct SimplePoint {
    CCPoint pos;
    ccColor3B color;

    bool operator==(const SimplePoint& other) const {
        return pos.equals(other.pos) && color.r == other.color.r &&
               color.g == other.color.g && color.b == other.color.b;
    }
};

namespace std {
    template <>
    struct hash<SimplePoint> {
        size_t operator()(const SimplePoint& point) const {
            return hash<float>()(point.pos.x) ^ hash<float>()(point.pos.y) ^
                   hash<int>()(point.color.r) ^ hash<int>()(point.color.g) ^
                   hash<int>()(point.color.b);
        }
    };
}

struct GradientConfig {

    std::vector<SimplePoint> points;
    bool isLinear = true;

    bool operator==(const GradientConfig& other) const {
        return isLinear == other.isLinear && points == other.points/* && isSecondary == other.isSecondary*/;
    }
    
    bool isEmpty(ColorType, bool);

};

struct Gradient {

    GradientConfig main;
    GradientConfig secondary;
    GradientConfig glow;
    GradientConfig white;
    GradientConfig line;

};

class FakeSpriteBatchNode : public CCSpriteBatchNode {

public:

    void draw() override {
        CCNode::draw();
    }

    void visit() override {
        CCNode::visit();
    }
    
};