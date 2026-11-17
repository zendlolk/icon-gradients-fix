#include "BarGlowLayer.hpp"
#include "GradientLayer.hpp"

#include "Utils/Utils.hpp"

// The power sliders go from a gentle curve to a steep one.
static float sliderToPower(float value) {
    return 1.2f + std::clamp(value, 0.f, 1.f) * 4.8f;
}

static float powerToSlider(float power) {
    return std::clamp((power - 1.2f) / 4.8f, 0.f, 1.f);
}

static std::string percentText(float value) {
    return fmt::format("{}%", static_cast<int>(std::round(value * 100.f)));
}

static const CCSize GRAPH_SIZE = { 62.f, 44.f };

BarGlowLayer* BarGlowLayer::create(GradientLayer* layer) {
    BarGlowLayer* ret = new BarGlowLayer();

    ret->m_layer = layer;

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

CCMenuItemToggler* BarGlowLayer::addToggle(const char* text, float y, bool on) {
    CCMenuItemToggler* toggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(BarGlowLayer::onToggle), 0.5f);
    toggle->setPosition({24, y});
    toggle->toggle(on);

    m_buttonMenu->addChild(toggle);

    CCLabelBMFont* label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(0.35f);
    label->setAnchorPoint({0.f, 0.5f});
    label->setPosition({38, y});

    m_mainLayer->addChild(label);

    return toggle;
}

// A button that reads "linear" or "exponential" and switches when clicked.
CCMenuItemToggler* BarGlowLayer::addCurveToggle(const char* text, float y, bool exponential) {
    CCLabelBMFont* label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(0.27f);
    label->setOpacity(170);
    label->setAnchorPoint({0.f, 0.5f});
    label->setPosition({38, y});

    m_mainLayer->addChild(label);

    CCMenuItemToggler* toggle = CCMenuItemToggler::create(
        ButtonSprite::create("linear", "bigFont.fnt", "GJ_button_04.png", 0.5f),
        ButtonSprite::create("exponential", "bigFont.fnt", "GJ_button_02.png", 0.5f),
        this,
        menu_selector(BarGlowLayer::onToggle)
    );
    toggle->setScale(0.6f);
    toggle->setPosition({172, y});
    toggle->toggle(exponential);

    m_buttonMenu->addChild(toggle);

    return toggle;
}

Slider* BarGlowLayer::addSlider(const char* text, float y, float value, CCLabelBMFont*& valueLabel, std::vector<CCNode*>* group) {
    CCLabelBMFont* label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(0.27f);
    label->setOpacity(170);
    label->setAnchorPoint({0.f, 0.5f});
    label->setPosition({38, y});

    m_mainLayer->addChild(label);

    Slider* slider = Slider::create(this, menu_selector(BarGlowLayer::onSlider), 0.5f);
    slider->setPosition({172, y});
    slider->setValue(std::clamp(value, 0.f, 1.f));

    m_mainLayer->addChild(slider);

    valueLabel = CCLabelBMFont::create("", "bigFont.fnt");
    valueLabel->setScale(0.27f);
    valueLabel->setAnchorPoint({0.f, 0.5f});
    valueLabel->setPosition({232, y});

    m_mainLayer->addChild(valueLabel);

    if (group) {
        group->push_back(label);
        group->push_back(slider);
        group->push_back(valueLabel);
    }

    return slider;
}

// Two small sliders on one row, for the bar's brightness and saturation.
void BarGlowLayer::addLookRow(float y, const char* firstText, float first, Slider*& firstSlider, CCLabelBMFont*& firstLabel, const char* secondText, float second, Slider*& secondSlider, CCLabelBMFont*& secondLabel) {
    auto add = [&](const char* text, float x, float value, Slider*& slider, CCLabelBMFont*& valueLabel) {
        CCLabelBMFont* label = CCLabelBMFont::create(text, "bigFont.fnt");
        label->setScale(0.27f);
        label->setOpacity(170);
        label->setAnchorPoint({0.f, 0.5f});
        label->setPosition({x, y});

        m_mainLayer->addChild(label);

        slider = Slider::create(this, menu_selector(BarGlowLayer::onSlider), 0.3f);
        slider->setPosition({x + 82.f, y});
        slider->setValue(std::clamp(value, 0.f, 1.f));

        m_mainLayer->addChild(slider);

        valueLabel = CCLabelBMFont::create("", "bigFont.fnt");
        valueLabel->setScale(0.25f);
        valueLabel->setAnchorPoint({0.f, 0.5f});
        valueLabel->setPosition({x + 118.f, y});

        m_mainLayer->addChild(valueLabel);
    };

    add(firstText, 38.f, first, firstSlider, firstLabel);
    add(secondText, 196.f, second, secondSlider, secondLabel);
}

// A small box showing the shape of a curve, centred on `y`.
CCDrawNode* BarGlowLayer::addGraph(float y, std::vector<CCNode*>& group) {
    NineSlice* bg = NineSlice::create("square02b_001.png");
    bg->setColor({0, 0, 0});
    bg->setOpacity(70);
    bg->setContentSize((GRAPH_SIZE + CCSize{ 8.f, 8.f }) * 2.f);
    bg->setScale(0.5f);
    bg->setPosition({336, y});

    m_mainLayer->addChild(bg);

    CCDrawNode* graph = CCDrawNode::create();
    graph->setPosition({336 - GRAPH_SIZE.width / 2.f, y - GRAPH_SIZE.height / 2.f});

    m_mainLayer->addChild(graph);

    group.push_back(bg);
    group.push_back(graph);

    return graph;
}

// Plots how far along the effect is (up) against the level (across).
void BarGlowLayer::drawGraph(CCDrawNode* graph, float power) {
    graph->clear();

    const int steps = 24;

    CCPoint previous = ccp(0.f, 0.f);

    for (int i = 1; i <= steps; i++) {
        float x = static_cast<float>(i) / steps;

        CCPoint point = ccp(x * GRAPH_SIZE.width, std::pow(x, power) * GRAPH_SIZE.height);

        graph->drawSegment(previous, point, 0.8f, ccc4f(1.f, 1.f, 1.f, 1.f));

        previous = point;
    }
}

bool BarGlowLayer::init() {
    Popup::init(380, 300);

    setTitle("Progress Bar Glow");

    m_effects = Utils::getBarGlowEffects();

    m_buildUpToggle = addToggle("Build up as the bar fills", 256, m_effects.buildUp);
    m_buildCurveToggle = addCurveToggle("growth", 238, m_effects.buildExponential);
    m_buildPowerSlider = addSlider("steepness", 220, powerToSlider(m_effects.buildPower), m_buildPowerLabel, &m_buildCurveNodes);
    m_buildGraph = addGraph(232, m_buildCurveNodes);
    addLookRow(
        202,
        "darken", m_effects.buildDim, m_buildDimSlider, m_buildDimLabel,
        "grey", m_effects.buildDesaturate, m_buildDesaturateSlider, m_buildDesaturateLabel
    );

    m_flashToggle = addToggle("Flash", 180, m_effects.flash);
    m_flashStartSlider = addSlider("starts at", 162, m_effects.flashStart, m_flashStartLabel);
    m_flashSpeedSlider = addSlider("speed", 144, m_effects.flashSpeed, m_flashSpeedLabel);
    m_flashIntensitySlider = addSlider("intensity", 126, m_effects.flashIntensity, m_flashIntensityLabel);
    m_flashCurveToggle = addCurveToggle("speed-up", 108, m_effects.flashExponential);
    m_flashPowerSlider = addSlider("steepness", 90, powerToSlider(m_effects.flashPower), m_flashPowerLabel, &m_flashCurveNodes);
    m_flashGraph = addGraph(110, m_flashCurveNodes);
    addLookRow(
        72,
        "brighten", m_effects.flashBrighten, m_flashBrightenSlider, m_flashBrightenLabel,
        "saturate", m_effects.flashSaturate, m_flashSaturateSlider, m_flashSaturateLabel
    );

    m_startPosToggle = addToggle("Flash from start positions", 50, m_effects.flashFromStartPos);
    m_startPosOffsetSlider = addSlider("starts after", 32, m_effects.startPosOffset, m_startPosOffsetLabel);

    CCLabelBMFont* hint = CCLabelBMFont::create("of the level, counted from the start position", "chatFont.fnt");
    hint->setScale(0.45f);
    hint->setOpacity(150);
    hint->setPosition({190, 16});

    m_mainLayer->addChild(hint);

    updateLabels();

    return true;
}

void BarGlowLayer::updateLabels() {
    m_buildPowerLabel->setString(fmt::format("{:.1f}", m_effects.buildPower).c_str());
    m_flashStartLabel->setString(percentText(m_effects.flashStart).c_str());
    m_flashSpeedLabel->setString(fmt::format("up to {}/s", static_cast<int>(std::round(2.f + m_effects.flashSpeed * 14.f))).c_str());
    m_flashIntensityLabel->setString(percentText(m_effects.flashIntensity).c_str());
    m_flashPowerLabel->setString(fmt::format("{:.1f}", m_effects.flashPower).c_str());
    m_startPosOffsetLabel->setString(percentText(m_effects.startPosOffset).c_str());
    m_buildDimLabel->setString(percentText(m_effects.buildDim).c_str());
    m_buildDesaturateLabel->setString(percentText(m_effects.buildDesaturate).c_str());
    m_flashBrightenLabel->setString(fmt::format("+{}", percentText(m_effects.flashBrighten)).c_str());
    m_flashSaturateLabel->setString(fmt::format("+{}", percentText(m_effects.flashSaturate)).c_str());

    // The curve settings only appear once a curve is chosen.
    for (CCNode* node : m_buildCurveNodes)
        node->setVisible(m_effects.buildExponential);

    for (CCNode* node : m_flashCurveNodes)
        node->setVisible(m_effects.flashExponential);

    drawGraph(m_buildGraph, m_effects.buildPower);
    drawGraph(m_flashGraph, m_effects.flashPower);
}

void BarGlowLayer::apply() {
    Utils::saveBarGlowEffects(m_effects);

    updateLabels();

    if (m_layer)
        m_layer->barGlowChanged();
}

void BarGlowLayer::onToggle(CCObject* sender) {
    CCMenuItemToggler* toggle = static_cast<CCMenuItemToggler*>(sender);

    // A toggler only flips after its callback.
    bool on = !toggle->isToggled();

    if (toggle == m_buildUpToggle) m_effects.buildUp = on;
    if (toggle == m_flashToggle) m_effects.flash = on;
    if (toggle == m_startPosToggle) m_effects.flashFromStartPos = on;
    if (toggle == m_buildCurveToggle) m_effects.buildExponential = on;
    if (toggle == m_flashCurveToggle) m_effects.flashExponential = on;

    apply();
}

void BarGlowLayer::onSlider(CCObject*) {
    m_effects.buildPower = sliderToPower(m_buildPowerSlider->getValue());
    m_effects.flashStart = std::clamp(m_flashStartSlider->getValue(), 0.f, 1.f);
    m_effects.flashSpeed = std::clamp(m_flashSpeedSlider->getValue(), 0.f, 1.f);
    m_effects.flashIntensity = std::clamp(m_flashIntensitySlider->getValue(), 0.f, 1.f);
    m_effects.flashPower = sliderToPower(m_flashPowerSlider->getValue());
    m_effects.startPosOffset = std::clamp(m_startPosOffsetSlider->getValue(), 0.f, 1.f);
    m_effects.buildDim = std::clamp(m_buildDimSlider->getValue(), 0.f, 1.f);
    m_effects.buildDesaturate = std::clamp(m_buildDesaturateSlider->getValue(), 0.f, 1.f);
    m_effects.flashBrighten = std::clamp(m_flashBrightenSlider->getValue(), 0.f, 1.f);
    m_effects.flashSaturate = std::clamp(m_flashSaturateSlider->getValue(), 0.f, 1.f);

    apply();
}
