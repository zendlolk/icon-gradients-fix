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

// Which corners of the game's quads sit across the trail from each other:
// 0 when it is corners 0-1 and 2-3, 1 when it is 1-2 and 3-0, -1 until known.
// This is a property of the game's code, so it only has to be found once.
static int s_cornerPairing = -1;

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
    static std::vector<std::array<CCPoint, 4>> quads;
    static std::vector<std::array<CCPoint, 4>> wide;
    static std::vector<TrailPiece> pieces;
    static std::vector<TrailEnd> ends;
    static std::vector<ccV2F_C4B_T2F> vertices;

    // Ask the game where the trail is, without drawing it. If it has nothing
    // to draw, let it handle the frame itself.
    if (!captureStroke(dt, m_waveSize, quads))
        return HardStreak::updateStroke(0.f);

    // Work out the corner pairing by asking again at a different width: the
    // midpoint of two corners across the trail is on its centre line, so it
    // stays put when the width changes. Done once, on a frame where the two
    // answers line up; until then the game draws its own trail.
    if (s_cornerPairing < 0) {
        int votes[2] = { 0, 0 };

        if (captureStroke(0.f, m_waveSize * 1.75f, wide) && wide.size() == quads.size())
            for (size_t i = 0; i < quads.size(); i++) {
                const std::array<CCPoint, 4>& a = quads[i];
                const std::array<CCPoint, 4>& b = wide[i];

                float moved01 = ccpDistance(midpoint(a[0], a[1]), midpoint(b[0], b[1]))
                    + ccpDistance(midpoint(a[2], a[3]), midpoint(b[2], b[3]));

                float moved12 = ccpDistance(midpoint(a[1], a[2]), midpoint(b[1], b[2]))
                    + ccpDistance(midpoint(a[3], a[0]), midpoint(b[3], b[0]));

                if (std::abs(moved01 - moved12) > 0.05f)
                    votes[moved01 < moved12 ? 0 : 1]++;
            }

        if (votes[0] != votes[1])
            s_cornerPairing = votes[0] > votes[1] ? 0 : 1;
        else
            return HardStreak::updateStroke(0.f);
    }

    pieces.clear();

    for (const std::array<CCPoint, 4>& a : quads) {
        TrailPiece piece = s_cornerPairing == 0
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

        // Pieces with no width or no length have no direction to work with.
        if (width < 0.01f || ccpDistance(piece.start, piece.end) < 0.05f) continue;

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
        // across the trail. Piece i owns ends 2i (start) and 2i + 1 (end).
        ends.clear();

        for (const TrailPiece& piece : pieces) {
            ends.push_back({ { piece.start, piece.startSide, piece.startSide * -1.f }, piece.end, false });
            ends.push_back({ { piece.end, piece.endSide, piece.endSide * -1.f }, piece.start, false });
        }

        // The game makes neighbouring pieces overlap at corners, which is
        // fine for a solid trail but leaves a glow doubled on one side and
        // missing on the other. So each piece is cut back to end exactly at
        // the corner, and the gap on the outside is filled with a rounded
        // join. On the inside the two pieces' glows overlap, which is what a
        // real glow does in a corner.
        //
        // The game hands the pieces over in order along the trail, so only
        // neighbours in that order can meet. Matching anything that is close
        // goes wrong when a piece is very short, such as when the wave
        // touches a surface for an instant.
        for (size_t piece = 0; piece + 1 < pieces.size(); piece++) {
            // The closest pair of free ends, one from each piece.
            TrailEnd* a = nullptr;
            TrailEnd* b = nullptr;

            float closest = 0.f;

            for (size_t i = piece * 2; i < piece * 2 + 2; i++)
                for (size_t j = piece * 2 + 2; j < piece * 2 + 4; j++) {
                    if (ends[i].joined || ends[j].joined) continue;

                    float distance = ccpDistance(ends[i].end.point, ends[j].end.point);

                    if (!a || distance < closest) {
                        a = &ends[i];
                        b = &ends[j];
                        closest = distance;
                    }
                }

            if (!a) continue;

            float reach = std::max(ccpLength(a->end.positive), ccpLength(b->end.positive)) * 4.f;

            // Too far apart to be the same corner: the trail has a break here.
            if (closest > reach) continue;

            a->joined = true;
            b->joined = true;

            CCPoint towardA = unit(a->opposite - a->end.point);
            CCPoint towardB = unit(b->opposite - b->end.point);

            // The corner is where the two centre lines cross. If they are
            // close to parallel the pieces just carry on from each other.
            float cross = towardA.x * towardB.y - towardA.y * towardB.x;

            CCPoint corner = midpoint(a->end.point, b->end.point);

            if (std::abs(cross) > 0.05f) {
                CCPoint between = b->end.point - a->end.point;
                float along = (between.x * towardB.y - between.y * towardB.x) / cross;

                CCPoint crossing = a->end.point + towardA * along;

                if (ccpDistance(crossing, corner) <= reach)
                    corner = crossing;
            }

            a->end.point = corner;
            b->end.point = corner;

            // Points into the corner, between the two pieces.
            CCPoint inward = towardA + towardB;

            if (ccpLength(inward) < 0.02f || std::abs(cross) <= 0.05f) continue;

            CCPoint outsideA = ccpDot(a->end.positive, inward) > 0.f ? a->end.negative : a->end.positive;
            CCPoint outsideB = ccpDot(b->end.positive, inward) > 0.f ? b->end.negative : b->end.positive;

            Utils::appendWaveFan(profile, corner, outsideA, outsideB, opacity, vertices);
        }

        for (size_t i = 0; i + 1 < ends.size(); i += 2) {
            // A piece cut back at both ends until nothing is left of it.
            if (ccpDot(ends[i + 1].end.point - ends[i].end.point, ends[i].opposite - ends[i + 1].opposite) <= 0.f)
                continue;

            Utils::appendWaveSegment(profile, ends[i].end, ends[i + 1].end, opacity, vertices);
        }

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
