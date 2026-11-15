#include "Utils/Utils.hpp"
#include "Utils/Cache.hpp"

#include "PlayLayer.hpp"
#include "PlayerObject.hpp"

void ProPlayLayer::resetLevel() {
    PlayLayer::resetLevel();

    // A new attempt: find out where it starts on the next frame.
    auto f = m_fields.self();

    f->m_runStart = -1.f;
    f->m_flashPhase = 0.f;
}

void ProPlayLayer::updateBarGradient(float dt) {
    auto f = m_fields.self();

    if (!m_progressFill || m_progressWidth <= 0.f || m_progressHeight <= 0.f) return;

    // The glow has to follow the bar as it fills.
    if (f->m_barGradientApplied) {
        if (!f->m_barGlow) return;

        float progress = std::clamp(m_progressFill->getTextureRect().size.width / m_progressWidth, 0.f, 1.f);

        if (f->m_runStart < 0.f)
            f->m_runStart = progress;

        float rate = Utils::getBarFlashRate(f->m_barEffects, progress, f->m_runStart, m_isTestMode);

        // Adding up the phase, rather than working it out from the time,
        // keeps the flashing smooth while its pace changes.
        if (rate > 0.f)
            f->m_flashPhase = std::fmod(f->m_flashPhase + rate * dt, 1.f);
        else
            f->m_flashPhase = 0.f;

        Utils::setBarGlowState(
            f->m_barGlow,
            progress,
            Utils::getBarGlowStrength(f->m_barEffects, progress, f->m_flashPhase, rate > 0.f)
        );

        return;
    }

    f->m_barGradientApplied = true;

    // The fill grows by widening its texture rect, so span the gradient over
    // the rect of the full bar instead of whatever is currently filled.
    float scale = CCDirector::get()->getContentScaleFactor();
    CCPoint origin = m_progressFill->getTextureRect().origin;

    CCRect fullRect = CCRect(
        origin.x * scale,
        origin.y * scale,
        m_progressWidth * scale,
        m_progressHeight * scale
    );

    Utils::applyBarGradient(m_progressFill, Utils::getSavedBarConfig(), true, &fullRect);

    f->m_barEffects = Utils::getBarGlowEffects();

    f->m_barGlow = Utils::updateBarGlow(
        m_progressFill,
        CCRect(origin.x, origin.y, m_progressWidth, m_progressHeight),
        Utils::getSavedBarConfig(),
        true,
        Utils::getGlow(GlowTarget::Bar, false)
    );

    // Until the next frame works out the real values.
    Utils::setBarGlowState(f->m_barGlow, 0.f, f->m_barEffects.buildUp ? 0.f : 1.f);
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