#pragma once

#include "Includes.hpp"

class GradientLayer;

// Options for how the progress bar's glow behaves while playing.
class BarGlowLayer : public Popup {

private:

    GradientLayer* m_layer = nullptr;

    BarGlowEffects m_effects;

    CCMenuItemToggler* m_buildUpToggle = nullptr;
    CCMenuItemToggler* m_flashToggle = nullptr;
    CCMenuItemToggler* m_startPosToggle = nullptr;

    // Switch between a straight line and a curve.
    CCMenuItemToggler* m_buildCurveToggle = nullptr;
    CCMenuItemToggler* m_flashCurveToggle = nullptr;

    Slider* m_buildPowerSlider = nullptr;
    Slider* m_flashStartSlider = nullptr;
    Slider* m_flashSpeedSlider = nullptr;
    Slider* m_flashIntensitySlider = nullptr;
    Slider* m_flashPowerSlider = nullptr;
    Slider* m_startPosOffsetSlider = nullptr;

    CCLabelBMFont* m_buildPowerLabel = nullptr;
    CCLabelBMFont* m_flashStartLabel = nullptr;
    CCLabelBMFont* m_flashSpeedLabel = nullptr;
    CCLabelBMFont* m_flashIntensityLabel = nullptr;
    CCLabelBMFont* m_flashPowerLabel = nullptr;
    CCLabelBMFont* m_startPosOffsetLabel = nullptr;

    // How much the bar and its glow brighten and saturate while flashing.
    Slider* m_flashBrightenSlider = nullptr;
    Slider* m_flashSaturateSlider = nullptr;

    CCLabelBMFont* m_flashBrightenLabel = nullptr;
    CCLabelBMFont* m_flashSaturateLabel = nullptr;

    CCDrawNode* m_buildGraph = nullptr;
    CCDrawNode* m_flashGraph = nullptr;

    // Only shown while the matching curve is exponential.
    std::vector<CCNode*> m_buildCurveNodes;
    std::vector<CCNode*> m_flashCurveNodes;

    bool init() override;

    CCMenuItemToggler* addToggle(const char*, float, bool);
    CCMenuItemToggler* addCurveToggle(const char*, float, bool);
    Slider* addSlider(const char*, float, float, CCLabelBMFont*&, std::vector<CCNode*>* = nullptr);
    CCDrawNode* addGraph(float, std::vector<CCNode*>&);
    void addLookRow(float, const char*, float, Slider*&, CCLabelBMFont*&, const char*, float, Slider*&, CCLabelBMFont*&);

    void drawGraph(CCDrawNode*, float);

    void onToggle(CCObject*);
    void onSlider(CCObject*);

    void updateLabels();
    void apply();

public:

    static BarGlowLayer* create(GradientLayer*);

};
