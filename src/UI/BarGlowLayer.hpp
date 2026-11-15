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

    bool init() override;

    CCMenuItemToggler* addToggle(const char*, float, bool);
    Slider* addSlider(const char*, float, float, CCLabelBMFont*&);

    void onToggle(CCObject*);
    void onSlider(CCObject*);

    void updateLabels();
    void apply();

public:

    static BarGlowLayer* create(GradientLayer*);

};
