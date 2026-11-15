#include "Utils/Utils.hpp"
#include "Utils/Cache.hpp"

#include "PlayLayer.hpp"
#include "PlayerObject.hpp"

void ProPlayLayer::updateBarGradient() {
    auto f = m_fields.self();

    if (!m_progressFill || m_progressWidth <= 0.f || m_progressHeight <= 0.f) return;

    // The glow has to follow the bar as it fills.
    if (f->m_barGradientApplied) {
        if (f->m_barGlow)
            Utils::setBarGlowFill(f->m_barGlow, m_progressFill->getTextureRect().size.width / m_progressWidth);

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

    f->m_barGlow = Utils::updateBarGlow(
        m_progressFill,
        CCRect(origin.x, origin.y, m_progressWidth, m_progressHeight),
        Utils::getSavedBarConfig(),
        true,
        Utils::getGlow(GlowTarget::Bar, false)
    );
}

void ProPlayLayer::postUpdate(float dt) {
    PlayLayer::postUpdate(dt);

    if (Utils::isSettingEnabled(MOD_DISABLED)) return;

    updateBarGradient();

    if (m_player1) {
        static_cast<ProPlayerObject*>(m_player1)->updateVisibility();
        static_cast<ProPlayerObject*>(m_player1)->updateFlip(0.f);
    }

    if (m_player2) {
        static_cast<ProPlayerObject*>(m_player2)->updateVisibility();
        static_cast<ProPlayerObject*>(m_player2)->updateFlip(0.f);
    }
}