#pragma once

#include "Includes.hpp"

#include <Geode/modify/HardStreak.hpp>

class $modify(ProHardStreak, HardStreak) {

    struct Fields {
        CCDrawNode* m_glow = nullptr;
        bool m_glowDrawn = false;
    };

    void clearGlow();

    void updateGlow(const std::vector<WaveGlowRing>&);

    void updateStroke(float);

};
