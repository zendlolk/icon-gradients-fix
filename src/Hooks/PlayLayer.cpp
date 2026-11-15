#include "Utils/Utils.hpp"
#include "Utils/Cache.hpp"

#include "PlayLayer.hpp"
#include "PlayerObject.hpp"

void ProPlayLayer::updateBarGradient() {
    auto f = m_fields.self();

    if (f->m_barGradientApplied || !m_progressFill) return;
    if (m_progressWidth <= 0.f || m_progressHeight <= 0.f) return;

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