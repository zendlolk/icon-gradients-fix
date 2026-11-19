#pragma once

#include "Includes.hpp"

class GradientLayer;

// Settings for how the progress bar changes while playing, on two pages:
// colour shifting and flashing, then what happens when the level is done.
class BarGlowLayer : public Popup {

private:

    GradientLayer* m_layer = nullptr;

    BarGlowEffects m_effects;

    std::vector<CCNode*> m_pages[2];

    CCMenuItemSpriteExtra* m_nextPageButton = nullptr;
    CCMenuItemSpriteExtra* m_prevPageButton = nullptr;

    // First page.
    CCMenuItemToggler* m_shiftToggle = nullptr;
    CCMenuItemToggler* m_buildCurveToggle = nullptr;
    CCMenuItemToggler* m_glowBuildToggle = nullptr;
    CCMenuItemToggler* m_flashToggle = nullptr;
    CCMenuItemToggler* m_flashCurveToggle = nullptr;
    CCMenuItemToggler* m_startPosToggle = nullptr;

    Slider* m_buildPowerSlider = nullptr;
    Slider* m_flashStartSlider = nullptr;
    Slider* m_flashSpeedSlider = nullptr;
    Slider* m_flashIntensitySlider = nullptr;
    Slider* m_flashPowerSlider = nullptr;
    Slider* m_flashBrightenSlider = nullptr;
    Slider* m_flashSaturateSlider = nullptr;
    Slider* m_startPosOffsetSlider = nullptr;

    CCLabelBMFont* m_buildPowerLabel = nullptr;
    CCLabelBMFont* m_flashStartLabel = nullptr;
    CCLabelBMFont* m_flashSpeedLabel = nullptr;
    CCLabelBMFont* m_flashIntensityLabel = nullptr;
    CCLabelBMFont* m_flashPowerLabel = nullptr;
    CCLabelBMFont* m_flashBrightenLabel = nullptr;
    CCLabelBMFont* m_flashSaturateLabel = nullptr;
    CCLabelBMFont* m_startPosOffsetLabel = nullptr;

    CCDrawNode* m_buildGraph = nullptr;
    CCDrawNode* m_flashGraph = nullptr;

    // Only shown while the matching curve is exponential.
    std::vector<CCNode*> m_buildCurveNodes;
    std::vector<CCNode*> m_flashCurveNodes;

    // Second page.
    CCMenuItemToggler* m_completeToggle = nullptr;
    CCMenuItemToggler* m_completeFlashToggle = nullptr;
    CCMenuItemToggler* m_animButtons[3] = { nullptr, nullptr, nullptr };

    Slider* m_completeFadeSlider = nullptr;
    Slider* m_animSpeedSlider = nullptr;
    Slider* m_animSizeSlider = nullptr;

    CCLabelBMFont* m_completeFadeLabel = nullptr;
    CCLabelBMFont* m_animSpeedLabel = nullptr;
    CCLabelBMFont* m_animSizeLabel = nullptr;

    bool init() override;

    void buildEffectsPage();
    void buildCompletePage();

    std::vector<CCNode*> collectChildren();

    CCMenuItemToggler* addToggle(const char*, float, bool, bool = false);
    CCMenuItemToggler* addCurveToggle(const char*, float, bool);
    Slider* addSlider(const char*, float, float, CCLabelBMFont*&, float = 52.f, std::vector<CCNode*>* = nullptr);
    CCDrawNode* addGraph(float, std::vector<CCNode*>&);
    void addLookRow(float, const char*, float, Slider*&, CCLabelBMFont*&, const char*, float, Slider*&, CCLabelBMFont*&);
    void addHint(const char*, float, float, bool = false);

    void drawGraph(CCDrawNode*, float);

    void showPage(int);

    void onPage(CCObject*);
    void onToggle(CCObject*);
    void onSlider(CCObject*);
    void onAnimButton(CCObject*);

    void updateAnimButtons();
    void updateLabels();
    void apply();

public:

    static BarGlowLayer* create(GradientLayer*);

};
