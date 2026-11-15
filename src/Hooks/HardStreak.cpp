#include "Utils/Utils.hpp"

#include "HardStreak.hpp"

// One straight piece of the trail as the game lays it out.
struct TrailPiece {
    CCPoint start;
    CCPoint end;
    CCPoint startSide;
    CCPoint endSide;
};

// One end of a piece, the piece's other end, and whether another piece
// meets it there.
struct TrailEnd {
    WaveEnd end;
    CCPoint opposite;
    bool joined;
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

// Direction of a vector, or nothing for one too short to have a direction.
static CCPoint unit(const CCPoint& v) {
    float length = ccpLength(v);

    return length > 0.0001f ? ccp(v.x / length, v.y / length) : ccp(0.f, 0.f);
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
    static std::vector<TrailEnd> ends;
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

    if (opacity > 0.f) {
        // Both ends of every piece, as the game laid them out: cut straight
        // across the trail.
        ends.clear();

        for (const TrailPiece& piece : pieces) {
            ends.push_back({ { piece.start, piece.startSide, piece.startSide * -1.f }, piece.end, false });
            ends.push_back({ { piece.end, piece.endSide, piece.endSide * -1.f }, piece.start, false });
        }

        // The game makes its pieces overlap at corners, which is fine for a
        // solid trail but doubles up a glow. So find where two pieces meet
        // and make them end exactly at the corner instead:
        //  - on the inside of the corner both are cut along the same slanted
        //    line, so they meet without overlapping;
        //  - on the outside both are cut straight, and the gap that leaves is
        //    filled with a rounded join.
        //
        // Pieces are matched by position rather than by order, since nothing
        // says which way round the game hands them over.
        for (size_t i = 0; i < ends.size(); i++) {
            TrailEnd& a = ends[i];

            if (a.joined) continue;

            float widthA = ccpLength(a.end.positive);

            // The nearest other end within reach is the one it meets. Taking
            // the nearest matters when pieces are short and several are close.
            size_t nearest = ends.size();
            float nearestDistance = 0.f;

            for (size_t j = i + 1; j < ends.size(); j++) {
                const TrailEnd& other = ends[j];

                // Two ends of the same piece sit next to each other in the list.
                if (other.joined || (j == i + 1 && i % 2 == 0)) continue;

                float distance = ccpDistance(a.end.point, other.end.point);

                if (distance > std::max(widthA, ccpLength(other.end.positive)) * 2.5f) continue;

                if (nearest == ends.size() || distance < nearestDistance) {
                    nearest = j;
                    nearestDistance = distance;
                }
            }

            if (nearest == ends.size()) continue;

            // A loop only so the cases below can leave early.
            for (;;) {
                TrailEnd& b = ends[nearest];

                float reach = std::max(widthA, ccpLength(b.end.positive)) * 2.5f;

                CCPoint towardA = unit(a.opposite - a.end.point);
                CCPoint towardB = unit(b.opposite - b.end.point);

                // The corner is where the two centre lines cross. If they are
                // close to parallel there is no corner to speak of.
                float cross = towardA.x * towardB.y - towardA.y * towardB.x;

                CCPoint corner = midpoint(a.end.point, b.end.point);

                if (std::abs(cross) > 0.05f) {
                    CCPoint between = b.end.point - a.end.point;
                    float along = (between.x * towardB.y - between.y * towardB.x) / cross;

                    CCPoint crossing = a.end.point + towardA * along;

                    if (ccpDistance(crossing, corner) <= reach)
                        corner = crossing;
                }

                a.joined = true;
                b.joined = true;

                a.end.point = corner;
                b.end.point = corner;

                // Points into the corner, between the two pieces.
                CCPoint inward = towardA + towardB;
                float inwardLength = ccpLength(inward);

                if (inwardLength < 0.02f || std::abs(cross) <= 0.05f) break;

                inward = inward * (1.f / inwardLength);

                // How far along that line a point has to go to be a given
                // distance from either piece's centre line. Limited, so a
                // very sharp corner does not shoot off into the distance.
                float slant = 1.f / std::max(0.34f, std::sqrt(std::max(0.f, 1.f - inwardLength * inwardLength / 4.f)));

                for (TrailEnd* end : { &a, &b }) {
                    float width = ccpLength(end->end.positive);
                    bool positiveIsInside = ccpDot(end->end.positive, inward) > 0.f;

                    (positiveIsInside ? end->end.positive : end->end.negative) = inward * (width * slant);
                }

                CCPoint outsideA = ccpDot(a.end.positive, inward) > 0.f ? a.end.negative : a.end.positive;
                CCPoint outsideB = ccpDot(b.end.positive, inward) > 0.f ? b.end.negative : b.end.positive;

                Utils::appendWaveFan(profile, corner, outsideA, outsideB, opacity, vertices);

                break;
            }
        }

        for (size_t i = 0; i + 1 < ends.size(); i += 2)
            Utils::appendWaveSegment(profile, ends[i].end, ends[i + 1].end, opacity, vertices);

        // Ends that meet nothing are rounded off.
        for (const TrailEnd& end : ends) {
            if (end.joined) continue;

            CCPoint outward = unit(end.end.point - end.opposite) * ccpLength(end.end.positive);

            Utils::appendWaveFan(profile, end.end.point, end.end.positive, outward, opacity, vertices);
            Utils::appendWaveFan(profile, end.end.point, outward, end.end.negative, opacity, vertices);
        }
    }

    Utils::setDrawNodeVertices(this, vertices);

    // Plain alpha blending: the vanilla additive blend would let the
    // background wash the colours out.
    setBlendFunc({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA });
}
