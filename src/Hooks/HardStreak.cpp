#include "Utils/Utils.hpp"

#include "HardStreak.hpp"

static float distanceToSegment(const CCPoint& point, const CCPoint& a, const CCPoint& b) {
    CCPoint ab = b - a;
    CCPoint ap = point - a;

    float lengthSq = ab.x * ab.x + ab.y * ab.y;
    float t = lengthSq > 0.0001f ? std::clamp((ap.x * ab.x + ap.y * ab.y) / lengthSq, 0.f, 1.f) : 0.f;

    float dx = ap.x - ab.x * t;
    float dy = ap.y - ab.y * t;

    return std::sqrt(dx * dx + dy * dy);
}

void ProHardStreak::clearGlow() {
    auto f = m_fields.self();

    if (!f->m_glow || !f->m_glowDrawn) return;

    f->m_glow->clear();
    f->m_glowDrawn = false;
}

// Expects this node's buffer to hold the trail at its full width, which is
// what the glow's size is measured from.
void ProHardStreak::updateGlow(const std::vector<WaveGlowRing>& rings) {
    if (rings.empty() || m_nBufferCount <= 0 || !m_pointArray)
        return clearGlow();

    std::vector<CCPoint> trail;

    auto addPoint = [&trail](const CCPoint& point) {
        if (trail.empty() || ccpDistance(trail.back(), point) > 0.01f)
            trail.push_back(point);
    };

    for (PointNode* node : CCArrayExt<PointNode*>(m_pointArray))
        addPoint(node->m_point);

    addPoint(m_currentPoint);

    if (trail.size() < 2)
        return clearGlow();

    // The trail's width is not stored anywhere, so take it from what the game
    // just drew: how far its vertices are from the trail's centre line.
    float halfWidth = 0.f;
    int checked = std::min<int>(m_nBufferCount, 96);

    for (int i = 0; i < checked; i++) {
        CCPoint vertex = { m_pBuffer[i].vertices.x, m_pBuffer[i].vertices.y };
        float closest = 100000.f;

        for (size_t j = 0; j + 1 < trail.size(); j++)
            closest = std::min(closest, distanceToSegment(vertex, trail[j], trail[j + 1]));

        halfWidth = std::max(halfWidth, closest);
    }

    // Anything outside this range means the trail is not shaped the way this
    // expects, and a wrong glow is worse than none.
    if (halfWidth < 0.5f || halfWidth > 60.f)
        return clearGlow();

    auto f = m_fields.self();

    if (!f->m_glow) {
        f->m_glow = CCDrawNode::create();
        f->m_glow->setID("wave-glow"_spr);

        // Additive: the glow only ever adds light, over the trail and around it.
        f->m_glow->setBlendFunc({ GL_ONE, GL_ONE });

        addChild(f->m_glow, 1);
    }

    std::vector<ccV2F_C4B_T2F> vertices;

    Utils::buildWaveGlowMesh(trail, halfWidth, rings, getOpacity() / 255.f, vertices);
    Utils::setDrawNodeVertices(f->m_glow, vertices);

    f->m_glowDrawn = !vertices.empty();
}

void ProHardStreak::updateStroke(float dt) {
    // Not drawing: the game keeps or clears its own trail, so only make sure
    // the glow does not outlive it.
    if (!m_drawStreak) {
        HardStreak::updateStroke(dt);

        if (m_nBufferCount <= 0 || getOpacity() == 0 || !m_pointArray || m_pointArray->count() == 0)
            clearGlow();

        return;
    }

    GJBaseGameLayer* gameLayer = GJBaseGameLayer::get();

    bool isFirstPlayer = gameLayer && gameLayer->m_player1 && gameLayer->m_player1->m_waveTrail == this;
    bool isSecondPlayer = gameLayer && gameLayer->m_player2 && gameLayer->m_player2->m_waveTrail == this;

    if (
        Utils::isSettingEnabled(MOD_DISABLED)
        || (!isFirstPlayer && !isSecondPlayer)
        || (isSecondPlayer && Utils::isSettingEnabled(P2_DISABLED))
    ) {
        HardStreak::updateStroke(dt);
        return clearGlow();
    }

    const WaveStyle& style = Utils::getWaveStyle(isSecondPlayer);

    if (style.bands.empty()) {
        HardStreak::updateStroke(dt);
        return updateGlow(style.glow);
    }

    // Let the game build the trail once per band at that band's width, and
    // keep the triangles it produced, recoloured. That way the shape always
    // matches whatever the game (or another mod) does with the trail.
    float waveSize = m_waveSize;
    GLubyte opacity = getOpacity();

    std::vector<ccV2F_C4B_T2F> vertices;

    bool first = true;

    for (const WaveBand& band : style.bands) {
        m_nBufferCount = 0;
        m_waveSize = waveSize * band.scale;

        HardStreak::updateStroke(first ? dt : 0.f);

        // The first band is the full-width one the glow measures from.
        if (first)
            updateGlow(style.glow);

        first = false;

        for (int i = 0; i < m_nBufferCount; i++) {
            ccV2F_C4B_T2F vertex = m_pBuffer[i];
            vertex.colors = ccc4(band.color.r, band.color.g, band.color.b, opacity);

            vertices.push_back(vertex);
        }
    }

    m_waveSize = waveSize;

    Utils::setDrawNodeVertices(this, vertices);

    // Solid trail: the vanilla additive blend would wash the colours out.
    setBlendFunc({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA });
}
