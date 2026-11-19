#include "Includes.hpp"

#include <Geode/modify/PlayLayer.hpp>

class $modify(ProPlayLayer, PlayLayer) {

    struct Fields {
        bool m_barGradientApplied = false;
        Ref<CCSprite> m_barGlow = nullptr;
        Ref<CCSprite> m_barOverlay = nullptr;
        Ref<CCSprite> m_barOverlayGlow = nullptr;
        Ref<CCSprite> m_barComplete = nullptr;
        Ref<CCSprite> m_barCompleteGlow = nullptr;

        // Seconds since the bar reached the end, or below 0.
        float m_completeTime = -1.f;
        // Seconds since the player died, or below 0.
        float m_deathTime = -1.f;

        // As of the last update, for deciding whether a death animates.
        float m_lastProgress = 0.f;
        bool m_lastFlashing = false;

        bool m_barTicking = false;

        // The pieces of a cracked or shattered bar, while they fall.
        Ref<CCNode> m_barPieces = nullptr;

        BarGlowEffects m_barEffects;

        // Where this attempt started (0 to 1), or below 0 until known.
        float m_runStart = -1.f;
        // Flashes since flashing began, counting fractions.
        float m_flashCycles = 0.f;
    };

    void updateBarGradient(float);
    void barTick(float);

    void destroyPlayer(PlayerObject*, GameObject*);
    void clearBarPieces();

    void resetLevel();

    void postUpdate(float);

};