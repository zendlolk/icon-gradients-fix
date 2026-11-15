#include "Utils/Utils.hpp"

#include "HardStreak.hpp"

void ProHardStreak::updateStroke(float dt) {
    if (!m_drawStreak || Utils::isSettingEnabled(MOD_DISABLED))
        return HardStreak::updateStroke(dt);

    GJBaseGameLayer* gameLayer = GJBaseGameLayer::get();

    if (!gameLayer)
        return HardStreak::updateStroke(dt);

    bool isFirstPlayer = gameLayer->m_player1 && gameLayer->m_player1->m_waveTrail == this;
    bool isSecondPlayer = gameLayer->m_player2 && gameLayer->m_player2->m_waveTrail == this;

    if (
        (!isFirstPlayer && !isSecondPlayer)
        || (isSecondPlayer && Utils::isSettingEnabled(P2_DISABLED))
    ) {
        return HardStreak::updateStroke(dt);
    }

    const std::vector<WaveLayer>& layers = Utils::getWaveLayers(isSecondPlayer);

    if (layers.empty())
        return HardStreak::updateStroke(dt);

    // Let the game build the trail once per layer at that layer's width, and
    // keep the triangles it produced, recoloured. That way the shape always
    // matches whatever the game (or another mod) does with the trail, and
    // the glow is part of the trail itself rather than something drawn
    // separately that could go missing or get left behind.
    float waveSize = m_waveSize;
    float opacity = getOpacity() / 255.f;

    // Reused between frames so drawing the trail does not allocate.
    static std::vector<ccV2F_C4B_T2F> vertices;

    vertices.clear();

    bool first = true;

    for (const WaveLayer& layer : layers) {
        m_nBufferCount = 0;
        m_waveSize = waveSize * layer.scale;

        HardStreak::updateStroke(first ? dt : 0.f);

        first = false;

        ccColor4B color = layer.color;
        color.a = static_cast<GLubyte>(color.a * opacity);

        for (int i = 0; i < m_nBufferCount; i++) {
            vertices.push_back(m_pBuffer[i]);
            vertices.back().colors = color;
        }
    }

    m_waveSize = waveSize;

    Utils::setDrawNodeVertices(this, vertices);

    // Plain alpha blending: the vanilla additive blend would let the
    // background wash the colours out.
    setBlendFunc({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA });
}
