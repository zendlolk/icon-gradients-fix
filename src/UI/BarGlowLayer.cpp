#include "BarGlowLayer.hpp"
#include "GradientLayer.hpp"

#include "Utils/Utils.hpp"

#include <unordered_set>

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

static const char* ANIM_NAMES[3] = { "none", "scroll", "shine" };

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

// A section toggle, or with `child` a smaller one that belongs to the
// section above it.
CCMenuItemToggler* BarGlowLayer::addToggle(const char* text, float y, bool on, bool child) {
    CCMenuItemToggler* toggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(BarGlowLayer::onToggle), child ? 0.38f : 0.5f);
    toggle->setPosition({child ? 46.f : 24.f, y});
    toggle->toggle(on);

    m_buttonMenu->addChild(toggle);

    CCLabelBMFont* label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(child ? 0.28f : 0.35f);
    label->setAnchorPoint({0.f, 0.5f});
    label->setPosition({child ? 57.f : 38.f, y});

    m_mainLayer->addChild(label);

    return toggle;
}

// A button that reads "linear" or "exponential" and switches when clicked.
CCMenuItemToggler* BarGlowLayer::addCurveToggle(const char* text, float y, bool exponential) {
    CCLabelBMFont* label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(0.27f);
    label->setOpacity(170);
    label->setAnchorPoint({0.f, 0.5f});
    label->setPosition({52, y});

    m_mainLayer->addChild(label);

    CCMenuItemToggler* toggle = CCMenuItemToggler::create(
        ButtonSprite::create("linear", "bigFont.fnt", "GJ_button_04.png", 0.5f),
        ButtonSprite::create("exponential", "bigFont.fnt", "GJ_button_02.png", 0.5f),
        this,
        menu_selector(BarGlowLayer::onToggle)
    );
    toggle->setScale(0.6f);
    toggle->setPosition({192, y});
    toggle->toggle(exponential);

    m_buttonMenu->addChild(toggle);

    return toggle;
}

Slider* BarGlowLayer::addSlider(const char* text, float y, float value, CCLabelBMFont*& valueLabel, float labelX, std::vector<CCNode*>* group) {
    CCLabelBMFont* label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(0.27f);
    label->setOpacity(170);
    label->setAnchorPoint({0.f, 0.5f});
    label->setPosition({labelX, y});

    m_mainLayer->addChild(label);

    Slider* slider = Slider::create(this, menu_selector(BarGlowLayer::onSlider), 0.5f);
    slider->setPosition({192, y});
    slider->setValue(std::clamp(value, 0.f, 1.f));

    m_mainLayer->addChild(slider);

    valueLabel = CCLabelBMFont::create("", "bigFont.fnt");
    valueLabel->setScale(0.27f);
    valueLabel->setAnchorPoint({0.f, 0.5f});
    valueLabel->setPosition({252, y});

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

    add(firstText, 52.f, first, firstSlider, firstLabel);
    add(secondText, 210.f, second, secondSlider, secondLabel);
}

void BarGlowLayer::addHint(const char* text, float x, float y, bool centred) {
    CCLabelBMFont* hint = CCLabelBMFont::create(text, "chatFont.fnt");
    hint->setScale(0.42f);
    hint->setOpacity(140);
    hint->setAnchorPoint({centred ? 0.5f : 0.f, 0.5f});
    hint->setPosition({x, y});

    m_mainLayer->addChild(hint);
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

// Everything currently in the popup, so each page can find what it added.
std::vector<CCNode*> BarGlowLayer::collectChildren() {
    std::vector<CCNode*> ret;

    for (CCNode* parent : { static_cast<CCNode*>(m_mainLayer), static_cast<CCNode*>(m_buttonMenu) })
        for (CCNode* child : parent->getChildrenExt<CCNode*>())
            ret.push_back(child);

    return ret;
}

bool BarGlowLayer::init() {
    Popup::init(380, 300);

    setTitle("Progress Bar Settings");

    m_effects = Utils::getBarGlowEffects();

    // Arrows just outside the popup switch pages.
    CCSprite* spr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    spr->setFlipX(true);
    spr->setScale(0.6f);

    m_nextPageButton = CCMenuItemSpriteExtra::create(spr, this, menu_selector(BarGlowLayer::onPage));
    m_nextPageButton->setPosition({m_size.width + 20.f, m_size.height / 2.f});

    m_buttonMenu->addChild(m_nextPageButton);

    spr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    spr->setScale(0.6f);

    m_prevPageButton = CCMenuItemSpriteExtra::create(spr, this, menu_selector(BarGlowLayer::onPage));
    m_prevPageButton->setPosition({-20.f, m_size.height / 2.f});

    m_buttonMenu->addChild(m_prevPageButton);

    for (int page = 0; page < 2; page++) {
        std::vector<CCNode*> before = collectChildren();
        std::unordered_set<CCNode*> existing(before.begin(), before.end());

        if (page == 0)
            buildEffectsPage();
        else
            buildCompletePage();

        for (CCNode* node : collectChildren())
            if (!existing.contains(node))
                m_pages[page].push_back(node);
    }

    updateChoices();
    updateLabels();
    showPage(0);

    return true;
}

void BarGlowLayer::buildEffectsPage() {
    m_shiftToggle = addToggle("Shift colours as the bar fills", 256, m_effects.shiftColours);
    m_buildCurveToggle = addCurveToggle("growth", 238, m_effects.buildExponential);
    m_buildPowerSlider = addSlider("steepness", 220, powerToSlider(m_effects.buildPower), m_buildPowerLabel, 52.f, &m_buildCurveNodes);
    m_buildGraph = addGraph(232, m_buildCurveNodes);
    m_glowBuildToggle = addToggle("glow build up", 202, m_effects.glowBuildUp, true);

    m_flashToggle = addToggle("Flash", 178, m_effects.flash);
    m_flashStartSlider = addSlider("starts at", 160, m_effects.flashStart, m_flashStartLabel);
    m_flashSpeedSlider = addSlider("speed", 142, m_effects.flashSpeed, m_flashSpeedLabel);
    m_flashIntensitySlider = addSlider("intensity", 124, m_effects.flashIntensity, m_flashIntensityLabel);
    m_flashCurveToggle = addCurveToggle("speed-up", 106, m_effects.flashExponential);
    m_flashPowerSlider = addSlider("steepness", 88, powerToSlider(m_effects.flashPower), m_flashPowerLabel, 52.f, &m_flashCurveNodes);
    m_flashGraph = addGraph(108, m_flashCurveNodes);
    addLookRow(
        70,
        "brighten", m_effects.flashBrighten, m_flashBrightenSlider, m_flashBrightenLabel,
        "saturate", m_effects.flashSaturate, m_flashSaturateSlider, m_flashSaturateLabel
    );

    m_startPosToggle = addToggle("from start positions", 50, m_effects.flashFromStartPos, true);
    m_startPosOffsetSlider = addSlider("starts after", 32, m_effects.startPosOffset, m_startPosOffsetLabel, 70.f);

    addHint("of the level, counted from the start position", 200, 16, true);
}

// One button of a row where only one can be picked. The tag says which row
// and which choice: anim 0-2, glow style 10-11, death 20-23.
CCMenuItemToggler* BarGlowLayer::addChoice(const char* text, float x, float y, int tag) {
    CCMenuItemToggler* button = CCMenuItemToggler::create(
        ButtonSprite::create(text, "bigFont.fnt", "GJ_button_04.png", 0.5f),
        ButtonSprite::create(text, "bigFont.fnt", "GJ_button_02.png", 0.5f),
        this,
        menu_selector(BarGlowLayer::onChoice)
    );
    button->setScale(0.55f);
    button->setPosition({x, y});
    button->setTag(tag);

    m_buttonMenu->addChild(button);

    return button;
}

void BarGlowLayer::addRowLabel(const char* text, float y) {
    CCLabelBMFont* label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(0.27f);
    label->setOpacity(170);
    label->setAnchorPoint({0.f, 0.5f});
    label->setPosition({52, y});

    m_mainLayer->addChild(label);
}

void BarGlowLayer::buildCompletePage() {
    m_completeToggle = addToggle("Change to colour 3 when complete", 258, m_effects.complete);

    addHint("Set colour 3 with the 3 button on the progress bar page.", 52, 242);

    m_completeFlashToggle = addToggle("flash white first", 226, m_effects.completeFlash, true);
    m_completeFadeSlider = addSlider("change time", 210, m_effects.completeFade, m_completeFadeLabel);

    addRowLabel("animation", 192);

    for (int i = 0; i < 3; i++)
        m_animButtons[i] = addChoice(ANIM_NAMES[i], 160.f + 56.f * i, 192, i);

    m_animSpeedSlider = addSlider("speed", 174, m_effects.animSpeed, m_animSpeedLabel);
    m_animSizeSlider = addSlider("size", 158, m_effects.animSize, m_animSizeLabel);

    // How the bar's glow falls off: evenly, or full next to the bar and then
    // dropping away fast.
    CCLabelBMFont* label = CCLabelBMFont::create("Glow style", "bigFont.fnt");
    label->setScale(0.35f);
    label->setAnchorPoint({0.f, 0.5f});
    label->setPosition({24, 134});

    m_mainLayer->addChild(label);

    m_glowStyleButtons[0] = addChoice("soft", 160.f, 134, 10);
    m_glowStyleButtons[1] = addChoice("tight", 216.f, 134, 11);

    label = CCLabelBMFont::create("When you die", "bigFont.fnt");
    label->setScale(0.35f);
    label->setAnchorPoint({0.f, 0.5f});
    label->setPosition({24, 110});

    m_mainLayer->addChild(label);

    static const char* DEATH_NAMES[4] = { "none", "crack", "shatter", "grey" };

    for (int i = 0; i < 4; i++)
        m_deathButtons[i] = addChoice(DEATH_NAMES[i], 82.f + 62.f * i, 90, 20 + i);

    m_deathFlashOnlyToggle = addToggle("only while the bar is flashing", 68, m_effects.deathFlashOnly, true);
    m_deathFromSlider = addSlider("otherwise from", 50, m_effects.deathFrom, m_deathFromLabel, 70.f);

    addHint("The bar's glow follows its colours and animations.", 200, 22, true);
}

void BarGlowLayer::showPage(int page) {
    for (int i = 0; i < 2; i++)
        for (CCNode* node : m_pages[i])
            node->setVisible(i == page);

    m_nextPageButton->setVisible(page == 0);
    m_prevPageButton->setVisible(page == 1);

    // The curve rows only show for an exponential curve.
    if (page == 0)
        updateLabels();
}

void BarGlowLayer::onPage(CCObject* sender) {
    showPage(sender == m_nextPageButton ? 1 : 0);
}

void BarGlowLayer::updateChoices() {
    for (int i = 0; i < 3; i++)
        m_animButtons[i]->toggle(i == m_effects.completeAnim);

    for (int i = 0; i < 2; i++)
        m_glowStyleButtons[i]->toggle(i == m_effects.glowStyle);

    for (int i = 0; i < 4; i++)
        m_deathButtons[i]->toggle(i == m_effects.deathStyle);
}

void BarGlowLayer::updateLabels() {
    m_buildPowerLabel->setString(fmt::format("{:.1f}", m_effects.buildPower).c_str());
    m_flashStartLabel->setString(percentText(m_effects.flashStart).c_str());
    m_flashSpeedLabel->setString(fmt::format("up to {}/s", static_cast<int>(std::round(2.f + m_effects.flashSpeed * 14.f))).c_str());
    m_flashIntensityLabel->setString(percentText(m_effects.flashIntensity).c_str());
    m_flashPowerLabel->setString(fmt::format("{:.1f}", m_effects.flashPower).c_str());
    m_startPosOffsetLabel->setString(percentText(m_effects.startPosOffset).c_str());
    m_flashBrightenLabel->setString(fmt::format("+{}", percentText(m_effects.flashBrighten)).c_str());
    m_flashSaturateLabel->setString(fmt::format("+{}", percentText(m_effects.flashSaturate)).c_str());

    m_completeFadeLabel->setString(fmt::format("{:.1f}s", Utils::getBarCompleteFade(m_effects)).c_str());
    m_animSpeedLabel->setString(percentText(m_effects.animSpeed).c_str());
    m_animSizeLabel->setString(percentText(m_effects.animSize).c_str());
    m_deathFromLabel->setString(percentText(m_effects.deathFrom).c_str());

    // The curve settings only appear once a curve is chosen, and only on the
    // page they belong to.
    bool firstPage = m_nextPageButton->isVisible();

    for (CCNode* node : m_buildCurveNodes)
        node->setVisible(firstPage && m_effects.buildExponential);

    for (CCNode* node : m_flashCurveNodes)
        node->setVisible(firstPage && m_effects.flashExponential);

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

    if (toggle == m_shiftToggle) m_effects.shiftColours = on;
    if (toggle == m_glowBuildToggle) m_effects.glowBuildUp = on;
    if (toggle == m_flashToggle) m_effects.flash = on;
    if (toggle == m_startPosToggle) m_effects.flashFromStartPos = on;
    if (toggle == m_buildCurveToggle) m_effects.buildExponential = on;
    if (toggle == m_flashCurveToggle) m_effects.flashExponential = on;
    if (toggle == m_completeToggle) m_effects.complete = on;
    if (toggle == m_completeFlashToggle) m_effects.completeFlash = on;
    if (toggle == m_deathFlashOnlyToggle) m_effects.deathFlashOnly = on;

    apply();
}

// Each row of buttons works as one choice.
void BarGlowLayer::onChoice(CCObject* sender) {
    int tag = static_cast<CCNode*>(sender)->getTag();

    if (tag >= 20)
        m_effects.deathStyle = std::clamp(tag - 20, 0, 3);
    else if (tag >= 10)
        m_effects.glowStyle = std::clamp(tag - 10, 0, 1);
    else
        m_effects.completeAnim = std::clamp(tag, 0, 2);

    // The clicked toggler flips itself after this, so set them all after.
    Loader::get()->queueInMainThread([self = Ref(this)] {
        self->updateChoices();
    });

    apply();
}

void BarGlowLayer::onSlider(CCObject*) {
    m_effects.buildPower = sliderToPower(m_buildPowerSlider->getValue());
    m_effects.flashStart = std::clamp(m_flashStartSlider->getValue(), 0.f, 1.f);
    m_effects.flashSpeed = std::clamp(m_flashSpeedSlider->getValue(), 0.f, 1.f);
    m_effects.flashIntensity = std::clamp(m_flashIntensitySlider->getValue(), 0.f, 1.f);
    m_effects.flashPower = sliderToPower(m_flashPowerSlider->getValue());
    m_effects.startPosOffset = std::clamp(m_startPosOffsetSlider->getValue(), 0.f, 1.f);
    m_effects.flashBrighten = std::clamp(m_flashBrightenSlider->getValue(), 0.f, 1.f);
    m_effects.flashSaturate = std::clamp(m_flashSaturateSlider->getValue(), 0.f, 1.f);

    m_effects.completeFade = std::clamp(m_completeFadeSlider->getValue(), 0.f, 1.f);
    m_effects.animSpeed = std::clamp(m_animSpeedSlider->getValue(), 0.f, 1.f);
    m_effects.animSize = std::clamp(m_animSizeSlider->getValue(), 0.f, 1.f);
    m_effects.deathFrom = std::clamp(m_deathFromSlider->getValue(), 0.f, 1.f);

    apply();
}
