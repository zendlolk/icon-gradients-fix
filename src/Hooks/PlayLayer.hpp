#include "Includes.hpp"

#include <Geode/modify/PlayLayer.hpp>

class $modify(ProPlayLayer, PlayLayer) {

    struct Fields {
        bool m_barGradientApplied = false;
        Ref<CCSprite> m_barGlow = nullptr;
        Ref<CCSprite> m_barOverlay = nullptr;
        Ref<CCSprite> m_barOverlayGlow = nullptr;

        BarGlowEffects m_barEffects;

        // Where this attempt started (0 to 1), or below 0 until known.
        float m_runStart = -1.f;
        // Flashes since flashing began, counting fractions.
        float m_flashCycles = 0.f;
    };

    void updateBarGradient(float);

    void resetLevel();

    void postUpdate(float);

};