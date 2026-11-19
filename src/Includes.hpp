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
    // The bar starts in its second colour and blends to its first as it fills.
    bool shiftColours = false;
    bool buildExponential = false;
    float buildPower = 2.f; // only when exponential: higher = later and steeper

    // Part of shifting colours: the glow grows from nothing along the same
    // curve.
    bool glowBuildUp = false;

    // Glow pulses, faster the closer the bar is to full.
    bool flash = false;
    float flashStart = 0.8f; // progress (0 to 1) where it begins
    float flashSpeed = 0.5f; // 0 to 1
    float flashIntensity = 0.7f; // 0 to 1, how far each flash dims the glow
    bool flashExponential = false;
    float flashPower = 2.f; // only when exponential: how late the pace picks up

    // How much brighter and more saturated the bar itself gets at the peak of
    // each flash. 0 to 1, where 1 doubles it.
    float flashBrighten = 0.f;
    float flashSaturate = 0.f;

    // From a start position: whether to flash at all, and how much of the
    // level has to be played first.
    bool flashFromStartPos = false;
    float startPosOffset = 0.5f;

    // When the level is complete the bar changes to its third colour.
    bool complete = false;
    bool completeFlash = true; // flash white first
    float completeFade = 0.3f; // 0 to 1: how long the change takes
    int completeAnim = 0; // 0 none, 1 scroll, 2 shine
    float animSpeed = 0.4f; // 0 to 1
    float animSize = 0.3f; // 0 to 1

    int glowStyle = 0; // 0 soft, 1 tight

    // What the bar does when the player dies.
    int deathStyle = 0; // 0 none, 1 crack, 2 shatter, 3 grey out
    bool deathFlashOnly = false; // only while the bar is flashing
    float deathFrom = 0.f; // otherwise from this much of the level, 0 to 1
};

// The sprites that make up a progress bar with this mod's effects: its fill,
// a copy of the fill in the bar's second colour that fades out as the bar
// builds up, and a glow for each.
struct BarSprites {
    CCSprite* fill = nullptr;
    CCSprite* overlay = nullptr;
    CCSprite* glow = nullptr;
    CCSprite* overlayGlow = nullptr;
    CCSprite* complete = nullptr;
    CCSprite* completeGlow = nullptr;
};

// Where a progress bar is, for its effects.
struct BarState {
    float progress = 0.f;
    float flashCycles = 0.f;
    bool flashing = false;
    // From the second colour (0) to the first (1), or below 0 to follow the
    // effects.
    float mix = -1.f;
    // Seconds since the level was completed, or below 0.
    float completeTime = -1.f;
    // Shows the completion colour whatever the settings, for editing it.
    bool forceComplete = false;
    // Seconds since the player died, or below 0.
    float deathTime = -1.f;
};

// One line of a crack across the progress bar, drawn growing from `from` to
// `to` between `start` and `end` seconds into the crack.
struct CrackSegment {
    CCPoint from;
    CCPoint to;
    float start;
    float end;
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