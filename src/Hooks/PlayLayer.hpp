#include "Includes.hpp"

#include <Geode/modify/PlayLayer.hpp>

class $modify(ProPlayLayer, PlayLayer) {

    struct Fields {
        bool m_barGradientApplied = false;
        Ref<CCSprite> m_barGlow = nullptr;

        BarGlowEffects m_barEffects;

        // Where this attempt started (0 to 1), or below 0 until known.
        float m_runStart = -1.f;
        float m_flashPhase = 0.f;
    };

    void updateBarGradient(float);

    void resetLevel();

    void postUpdate(float);

};