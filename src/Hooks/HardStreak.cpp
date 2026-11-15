#include "Utils/Utils.hpp"

#include "HardStreak.hpp"

// CCDrawNode::ensureCapacity is private, but it is the only safe way to grow
// the node's vertex buffer. This grants access to it without changing the
// class.
template <auto Member>
struct DrawNodeAccess {
    friend void ensureDrawNodeCapacity(CCDrawNode* node, unsigned int count) {
        (node->*Member)(count);
    }
};

template struct DrawNodeAccess<&CCDrawNode::ensureCapacity>;

void ensureDrawNodeCapacity(CCDrawNode*, unsigned int);

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

    const std::vector<WaveBand>& bands = Utils::getWaveBands();

    if (bands.empty())
        return HardStreak::updateStroke(dt);

    // Let the game build the trail once per band at that band's width, and
    // keep the triangles it produced, recoloured. That way the shape always
    // matches whatever the game (or another mod) does with the trail.
    float waveSize = m_waveSize;
    GLubyte opacity = getOpacity();

    std::vector<ccV2F_C4B_T2F> vertices;

    bool first = true;

    for (const WaveBand& band : bands) {
        m_nBufferCount = 0;
        m_waveSize = waveSize * band.scale;

        HardStreak::updateStroke(first ? dt : 0.f);

        first = false;

        for (int i = 0; i < m_nBufferCount; i++) {
            ccV2F_C4B_T2F vertex = m_pBuffer[i];
            vertex.colors = ccc4(band.color.r, band.color.g, band.color.b, opacity);

            vertices.push_back(vertex);
        }
    }

    m_waveSize = waveSize;
    m_nBufferCount = 0;

    if (!vertices.empty()) {
        ensureDrawNodeCapacity(this, static_cast<unsigned int>(vertices.size()));

        std::memcpy(m_pBuffer, vertices.data(), vertices.size() * sizeof(ccV2F_C4B_T2F));

        m_nBufferCount = static_cast<GLsizei>(vertices.size());
    }

    m_bDirty = true;

    // Solid trail: the vanilla additive blend would wash the colours out.
    setBlendFunc({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA });
}
