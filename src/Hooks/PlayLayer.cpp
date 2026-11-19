#include "Utils/Utils.hpp"
#include "Utils/Cache.hpp"

#include "PlayLayer.hpp"
#include "PlayerObject.hpp"

void ProPlayLayer::resetLevel() {
    PlayLayer::resetLevel();

    // A new attempt: find out where it starts on the next frame.
    auto f = m_fields.self();

    f->m_runStart = -1.f;
    f->m_flashCycles = 0.f;
    f->m_completeTime = -1.f;
}

void ProPlayLayer::updateBarGradient(float dt) {
    auto f = m_fields.self();

    if (!m_progressFill || m_progressWidth <= 0.f || m_progressHeight <= 0.f) return;

    // The glow and the effects follow the bar as it fills.
    if (f->m_barGradientApplied) {
        const BarGlowEffects& effects = f->m_barEffects;

        if (!f->m_barGlow && !effects.shiftColours && !effects.flash && !effects.complete) return;

        float progress = std::clamp(m_progressFill->getTextureRect().size.width / m_progressWidth, 0.f, 1.f);

        if (f->m_runStart < 0.f)
            f->m_runStart = progress;

        float rate = Utils::getBarFlashRate(f->m_barEffects, progress, f->m_runStart, m_isTestMode);

        // Adding up the flashes, rather than working them out from the time,
        // keeps the flashing smooth while its pace changes.
        if (rate > 0.f)
            f->m_flashCycles += rate * dt;
        else
            f->m_flashCycles = 0.f;

        // Once the bar is full the level is complete, until the next attempt.
        if (effects.complete) {
            if (f->m_completeTime >= 0.f)
                f->m_completeTime += dt;
            else if (progress >= 0.995f)
                f->m_completeTime = 0.f;
        }

        BarState state;

        state.progress = progress;
        state.flashCycles = f->m_flashCycles;
        state.flashing = rate > 0.f;
        state.completeTime = f->m_completeTime;

        Utils::updateBarEffects(
            {
                m_progressFill, f->m_barOverlay, f->m_barGlow, f->m_barOverlayGlow,
                f->m_barComplete, f->m_barCompleteGlow
            },
            effects, state
        );

        return;
    }

    f->m_barGradientApplied = true;

    // The fill grows by widening its texture rect, so the gradient spans the
    // rect of the full bar instead of whatever is currently filled.
    CCPoint origin = m_progressFill->getTextureRect().origin;

    BarSprites bar;

    Utils::setupBar(bar, m_progressFill, CCRect(origin.x, origin.y, m_progressWidth, m_progressHeight), true);

    f->m_barGlow = bar.glow;
    f->m_barOverlay = bar.overlay;
    f->m_barOverlayGlow = bar.overlayGlow;
    f->m_barComplete = bar.complete;
    f->m_barCompleteGlow = bar.completeGlow;
    f->m_barEffects = Utils::getBarGlowEffects();

    // Until the next frame works out the real values.
    Utils::updateBarEffects(bar, f->m_barEffects, BarState{});
}

void ProPlayLayer::postUpdate(float dt) {
    PlayLayer::postUpdate(dt);

    if (Utils::isSettingEnabled(MOD_DISABLED)) return;

    updateBarGradient(dt);

    if (m_player1) {
        static_cast<ProPlayerObject*>(m_player1)->updateVisibility();
        static_cast<ProPlayerObject*>(m_player1)->updateFlip(0.f);
    }

    if (m_player2) {
        static_cast<ProPlayerObject*>(m_player2)->updateVisibility();
        static_cast<ProPlayerObject*>(m_player2)->updateFlip(0.f);
    }
}