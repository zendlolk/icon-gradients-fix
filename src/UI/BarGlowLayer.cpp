#include "BarGlowLayer.hpp"
#include "GradientLayer.hpp"

#include "Utils/Utils.hpp"

// The curve sliders go from a straight line to a steep curve.
static float sliderToPower(float value) {
    return 1.f + std::clamp(value, 0.f, 1.f) * 5.f;
}

static float powerToSlider(float power) {
    return std::clamp((power - 1.f) / 5.f, 0.f, 1.f);
}

static std::string powerText(float power) {
    return power < 1.05f ? "linear" : fmt::format("power {:.1f}", power);
}

static std::string percentText(float value) {
    return fmt::format("{}%", static_cast<int>(std::round(value * 100.f)));
}

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

Slider* BarGlowLayer::addSlider(const char* text, float y, float value, CCLabelBMFont*& valueLabel) {
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

    return slider;
}

bool BarGlowLayer::init() {
    Popup::init(300, 262);

    setTitle("Progress Bar Glow");

    m_effects = Utils::getBarGlowEffects();

    m_buildUpToggle = addToggle("Build up as the bar fills", 216, m_effects.buildUp);
    m_buildPowerSlider = addSlider("curve", 196, powerToSlider(m_effects.buildPower), m_buildPowerLabel);

    m_flashToggle = addToggle("Flash", 168, m_effects.flash);
    m_flashStartSlider = addSlider("starts at", 148, m_effects.flashStart, m_flashStartLabel);
    m_flashSpeedSlider = addSlider("speed", 128, m_effects.flashSpeed, m_flashSpeedLabel);
    m_flashIntensitySlider = addSlider("intensity", 108, m_effects.flashIntensity, m_flashIntensityLabel);
    m_flashPowerSlider = addSlider("speed-up", 88, powerToSlider(m_effects.flashPower), m_flashPowerLabel);

    m_startPosToggle = addToggle("Flash from start positions", 60, m_effects.flashFromStartPos);
    m_startPosOffsetSlider = addSlider("starts after", 40, m_effects.startPosOffset, m_startPosOffsetLabel);

    CCLabelBMFont* hint = CCLabelBMFont::create("of the level, counted from the start position", "chatFont.fnt");
    hint->setScale(0.45f);
    hint->setOpacity(150);
    hint->setPosition({150, 22});

    m_mainLayer->addChild(hint);

    updateLabels();

    return true;
}

void BarGlowLayer::updateLabels() {
    m_buildPowerLabel->setString(powerText(m_effects.buildPower).c_str());
    m_flashStartLabel->setString(percentText(m_effects.flashStart).c_str());
    m_flashSpeedLabel->setString(fmt::format("up to {}/s", static_cast<int>(std::round(2.f + m_effects.flashSpeed * 14.f))).c_str());
    m_flashIntensityLabel->setString(percentText(m_effects.flashIntensity).c_str());
    m_flashPowerLabel->setString(powerText(m_effects.flashPower).c_str());
    m_startPosOffsetLabel->setString(percentText(m_effects.startPosOffset).c_str());
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

    apply();
}

void BarGlowLayer::onSlider(CCObject*) {
    m_effects.buildPower = sliderToPower(m_buildPowerSlider->getValue());
    m_effects.flashStart = std::clamp(m_flashStartSlider->getValue(), 0.f, 1.f);
    m_effects.flashSpeed = std::clamp(m_flashSpeedSlider->getValue(), 0.f, 1.f);
    m_effects.flashIntensity = std::clamp(m_flashIntensitySlider->getValue(), 0.f, 1.f);
    m_effects.flashPower = sliderToPower(m_flashPowerSlider->getValue());
    m_effects.startPosOffset = std::clamp(m_startPosOffsetSlider->getValue(), 0.f, 1.f);

    apply();
}
