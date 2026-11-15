#include "Utils/Utils.hpp"

#include "HardStreak.hpp"

// One straight piece of the trail as the game lays it out.
struct TrailPiece {
    CCPoint start;
    CCPoint end;
    CCPoint startSide;
    CCPoint endSide;
};

// While set, quads the given node asks to draw are recorded here instead of
// being drawn. Only ever set for the duration of a captureStroke call.
static CCDrawNode* s_captureNode = nullptr;
static std::vector<std::array<CCPoint, 4>>* s_captured = nullptr;

bool ProDrawNode::drawPolygon(CCPoint* verts, unsigned int count, const ccColor4F& fillColor, float borderWidth, const ccColor4F& borderColor, BorderAlignment alignment) {
    if (s_captureNode != this || !s_captured)
        return CCDrawNode::drawPolygon(verts, count, fillColor, borderWidth, borderColor, alignment);

    if (verts && count == 4)
        s_captured->push_back({ verts[0], verts[1], verts[2], verts[3] });

    return true;
}

// Runs the game's own trail code at a given width, recording the quads it
// would have drawn. Returns whether there were any.
bool ProHardStreak::captureStroke(float dt, float waveSize, std::vector<std::array<CCPoint, 4>>& quads) {
    quads.clear();

    float previousSize = m_waveSize;

    m_waveSize = waveSize;

    s_captureNode = this;
    s_captured = &quads;

    HardStreak::updateStroke(dt);

    s_captureNode = nullptr;
    s_captured = nullptr;

    m_waveSize = previousSize;

    return !quads.empty();
}

static CCPoint midpoint(const CCPoint& a, const CCPoint& b) {
    return ccp((a.x + b.x) / 2.f, (a.y + b.y) / 2.f);
}

static CCPoint halfDifference(const CCPoint& a, const CCPoint& b) {
    return ccp((a.x - b.x) / 2.f, (a.y - b.y) / 2.f);
}

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

    const std::vector<WaveNode>& profile = Utils::getWaveProfile(isSecondPlayer);

    if (profile.size() < 2)
        return HardStreak::updateStroke(dt);

    // Reused between frames, so drawing the trail does not allocate once
    // they have grown to size.
    static std::vector<std::array<CCPoint, 4>> narrow;
    static std::vector<std::array<CCPoint, 4>> wide;
    static std::vector<TrailPiece> pieces;
    static std::vector<ccV2F_C4B_T2F> vertices;

    // Ask the game where the trail is, at two widths, without drawing it.
    // Comparing the two shows which corners of each quad sit across the trail
    // from each other: their midpoint is on the trail's centre line, so it
    // does not move when the width changes.
    //
    // If nothing was recorded, or the two do not line up, fall back to the
    // game drawing its own trail rather than drawing something wrong.
    if (
        !captureStroke(dt, m_waveSize, narrow)
        || !captureStroke(0.f, m_waveSize * 1.75f, wide)
        || narrow.size() != wide.size()
    ) {
        return HardStreak::updateStroke(0.f);
    }

    pieces.clear();

    for (size_t i = 0; i < narrow.size(); i++) {
        const std::array<CCPoint, 4>& a = narrow[i];
        const std::array<CCPoint, 4>& b = wide[i];

        float moved01 = ccpDistance(midpoint(a[0], a[1]), midpoint(b[0], b[1]))
            + ccpDistance(midpoint(a[2], a[3]), midpoint(b[2], b[3]));

        float moved12 = ccpDistance(midpoint(a[1], a[2]), midpoint(b[1], b[2]))
            + ccpDistance(midpoint(a[3], a[0]), midpoint(b[3], b[0]));

        TrailPiece piece = moved01 <= moved12
            ? TrailPiece{
                midpoint(a[0], a[1]),
                midpoint(a[2], a[3]),
                halfDifference(a[0], a[1]),
                halfDifference(a[3], a[2])
            }
            : TrailPiece{
                midpoint(a[1], a[2]),
                midpoint(a[3], a[0]),
                halfDifference(a[1], a[2]),
                halfDifference(a[0], a[3])
            };

        float width = ccpLength(piece.startSide);

        if (width < 0.01f) continue;

        // The game can stack several quads of different widths on one piece
        // of trail. Only the widest is the trail's real outline.
        bool duplicate = false;

        for (TrailPiece& other : pieces) {
            if (ccpDistance(other.start, piece.start) > 0.05f || ccpDistance(other.end, piece.end) > 0.05f)
                continue;

            if (width > ccpLength(other.startSide))
                other = piece;

            duplicate = true;
            break;
        }

        if (!duplicate)
            pieces.push_back(piece);
    }

    vertices.clear();

    float opacity = getOpacity() / 255.f;

    if (opacity > 0.f)
        for (const TrailPiece& piece : pieces)
            Utils::appendWaveSegment(profile, piece.start, piece.end, piece.startSide, piece.endSide, opacity, vertices);

    Utils::setDrawNodeVertices(this, vertices);

    // Plain alpha blending: the vanilla additive blend would let the
    // background wash the colours out.
    setBlendFunc({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA });
}
