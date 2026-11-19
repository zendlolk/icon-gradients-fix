#include "Utils/Utils.hpp"

#include "PointsLayer.hpp"
#include "GradientLayer.hpp"

// #include <hiimjustin000.more_icons/include/MoreIcons.hpp>

PointsLayer* PointsLayer::create(const CCSize& size, GradientLayer* layer) {
    PointsLayer* ret = new PointsLayer();

    ret->m_layer = layer;

    if (ret->init(size)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
};

bool PointsLayer::init(CCSize size) {
    if (!CCLayer::init()) return false;

    m_shadow = CCSprite::create("shadow.png"_spr);
    m_shadow->setOpacity(49);
    m_shadow->setPosition({178.5f, 170});
    m_shadow->setScaleY(0.8f);

    m_icon = Utils::createIcon(IconType::Cube);
    m_icon->setPosition({178.5f, 200});
    m_icon->setScale(1.7f);

    addChild(m_shadow);
    addChild(m_icon);

    createBar();
    createWave();

    setPlayerFrame(IconType::Cube);

    setContentSize(size);
    setAnchorPoint({0, 0});
    setTouchEnabled(true);
    registerWithTouchDispatcher();
    setTouchMode(kCCTouchesOneByOne);
    
    return true;
}

// Preview of the in-game progress bar, built the same way the game builds it.
void PointsLayer::createBar() {
    CCSprite* groove = CCSprite::create("slidergroove2.png");
    CCSprite* fill = CCSprite::create("sliderBar2.png");

    if (!groove || !fill) return;

    CCSize grooveSize = groove->getContentSize();
    CCSize fillSize = { grooveSize.width - 4.f, 8.f };

    // Anchored on its left edge like the game's, so it can fill up in place.
    fill->setTextureRect({ 0.f, 0.f, fillSize.width, fillSize.height });
    fill->setColor(Utils::getPlayerColor(ColorType::Main, false));
    fill->setAnchorPoint({0.f, 0.5f});
    fill->setPosition({2.f, grooveSize.height / 2.f});

    m_barFullSize = fillSize;

    groove->setPosition(grooveSize / 2.f);

    m_bar = CCNode::create();
    m_bar->setContentSize(grooveSize);
    m_bar->setAnchorPoint({0.5f, 0.5f});
    m_bar->setPosition({178.5f, 226});
    m_bar->setScale(std::min(1.15f, 235.f / grooveSize.width));
    m_bar->setVisible(false);

    m_bar->addChild(fill, 1);
    m_bar->addChild(groove, 2);

    m_barFill = fill;

    addChild(m_bar);
}

// Preview of a straight piece of wave trail. Points on this page only move
// along the vertical line through its middle.
void PointsLayer::createWave() {
    CCSize size = { 150.f, 56.f };

    m_wave = CCNode::create();
    m_wave->setContentSize(size);
    m_wave->setAnchorPoint({0.5f, 0.5f});
    m_wave->setPosition({178.5f, 200});
    m_wave->setVisible(false);

    m_waveDraw = CCDrawNode::create();
    m_waveDraw->setBlendFunc({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA });

    m_wave->addChild(m_waveDraw);

    m_waveIcon = Utils::createIcon(IconType::Wave);
    m_waveIcon->setScale(1.6f);
    m_waveIcon->setPosition({size.width + 4.f, size.height / 2.f});

    Utils::setIconColors(m_waveIcon, ColorType::Main, false, false);

    m_wave->addChild(m_waveIcon, 1);

    addChild(m_wave);
}

// A bigger glow needs more room, so the previewed trail gets thinner as the
// glow grows. Runs before points are laid out, since they follow its size.
void PointsLayer::updateWaveLayout() {
    if (!m_wave) return;

    GlowSettings glow = Utils::getWaveGlow(m_layer->isSecondPlayer());

    float halfHeight = std::clamp(46.f / Utils::getWaveGlowReach(glow), 9.f, 28.f);

    m_wave->setContentSize({ 150.f, halfHeight * 2.f });
    m_waveIcon->setPosition({ 154.f, halfHeight });
}

bool PointsLayer::hasPage(EditorPage page) {
    switch (page) {
        case EditorPage::Bar: return m_barFill != nullptr;
        case EditorPage::Wave: return m_waveDraw != nullptr;
        default: return true;
    }
}

void PointsLayer::setPage(EditorPage page) {
    if (!hasPage(page)) return;

    m_page = page;

    if (m_bar) {
        m_bar->setVisible(page == EditorPage::Bar);

        // The bar preview plays through its glow effects while it is shown.
        unschedule(schedule_selector(PointsLayer::updateBarPreview));

        m_barPreviewTime = 0.f;
        m_barFlashCycles = 0.f;

        if (page == EditorPage::Bar)
            schedule(schedule_selector(PointsLayer::updateBarPreview));
        else
            setBarPreviewFill(1.f);
    }

    if (m_wave)
        m_wave->setVisible(page == EditorPage::Wave);

    m_icon->setVisible(page == EditorPage::Icons);
    m_shadow->setVisible(page == EditorPage::Icons);
}

// Whether the preview loop plays, and how fast (0 to 1).
void PointsLayer::setBarPreview(bool enabled, float speed) {
    m_barPreviewEnabled = enabled;
    m_barPreviewSpeed = std::clamp(speed, 0.f, 1.f);
}

// Starts the preview loop over from a point on the bar (0 to 1). The run
// still counts as a normal one from 0%, not as a start position.
void PointsLayer::restartBarPreview(float from) {
    m_barPreviewFrom = std::clamp(from, 0.f, 0.99f);
    m_barPreviewTime = 0.f;
    m_barFlashCycles = 0.f;
}

// The loop only plays when it is switched on and there is an effect to show,
// on the glow or on the bar itself.
bool PointsLayer::isBarPreviewPlaying() {
    return m_page == EditorPage::Bar
        && m_barPreviewEnabled
        && m_barFill
        && (m_barEffects.shiftColours || m_barEffects.flash || m_barEffects.complete);
}

// Lets the preview bar be grabbed and dragged to any point. `begin` is for a
// new touch, which only counts if it lands on the bar. The loop carries on
// from wherever the bar is let go.
bool PointsLayer::dragBarPreview(CCTouch* touch, bool begin) {
    if (!m_bar || !isBarPreviewPlaying()) return false;

    CCPoint pos = m_bar->convertToNodeSpace(touch->getLocation());
    CCSize size = m_bar->getContentSize();

    if (begin) {
        if (pos.x < -4.f || pos.x > size.width + 4.f || pos.y < -5.f || pos.y > size.height + 5.f)
            return false;

        m_barDragging = true;
    }

    if (!m_barDragging) return false;

    float progress = std::clamp((pos.x - 2.f) / std::max(1.f, m_barFullSize.width), 0.f, 1.f);
    float duration = 12.f - 10.5f * m_barPreviewSpeed;

    m_barPreviewTime = (progress - m_barPreviewFrom) * duration;

    return true;
}

// Shows the preview bar filled to `progress` with its glow at `strength`.
void PointsLayer::setBarPreviewFill(float progress, float flashCycles, bool flashing, float mix, float completeTime, bool forceComplete) {
    if (!m_barFill) return;

    m_barFill->setTextureRect({ 0.f, 0.f, m_barFullSize.width * progress, m_barFullSize.height });

    BarState state;

    state.progress = progress;
    state.flashCycles = flashCycles;
    state.flashing = flashing;
    state.mix = mix;
    state.completeTime = completeTime;
    state.forceComplete = forceComplete;

    Utils::updateBarEffects(
        { m_barFill, m_barOverlay, m_barGlow, m_barOverlayGlow, m_barComplete, m_barCompleteGlow },
        m_barEffects, state
    );
}

// With glow effects on, the preview bar fills up on a loop so the build-up
// and the flashing can be seen. Otherwise it just sits full.
void PointsLayer::updateBarPreview(float dt) {
    if (!m_barFill || m_page != EditorPage::Bar) return;

    m_barAnimClock = std::fmod(m_barAnimClock + dt, 600.f);

    if (!isBarPreviewPlaying()) {
        m_barPreviewTime = 0.f;
        m_barFlashCycles = 0.f;
        m_barDragging = false;

        // A full bar, in whichever colour is being edited. The completion
        // colour plays its animation.
        if (m_barEditing == 2)
            return setBarPreviewFill(1.f, 0.f, false, 1.f, 10.f + m_barAnimClock, true);

        return setBarPreviewFill(1.f, 0.f, false, m_barEditing == 1 ? 0.f : 1.f);
    }

    // Fills from the chosen starting point, holds while full, then loops.
    // A whole bar takes between 12 seconds and 1.5, by the speed setting.
    float duration = 12.f - 10.5f * m_barPreviewSpeed;
    float fillTime = duration * std::max(0.01f, 1.f - m_barPreviewFrom);

    // Long enough to see the level-complete change and its animation.
    float hold = m_barEffects.complete ? Utils::getBarCompleteFade(m_barEffects) + 3.f : 1.f;

    // Held in place while it is being dragged. The time can be below zero
    // after a drag to before the starting point; it just counts up from there.
    if (!m_barDragging) {
        m_barPreviewTime += dt;

        if (m_barPreviewTime >= fillTime + hold)
            m_barPreviewTime = 0.f;
    }

    float progress = std::clamp(m_barPreviewFrom + m_barPreviewTime / duration, 0.f, 1.f);
    float rate = Utils::getBarFlashRate(m_barEffects, progress, 0.f, false);

    if (rate > 0.f)
        m_barFlashCycles += rate * dt;
    else
        m_barFlashCycles = 0.f;

    // The level counts as complete from the moment the bar is full.
    float completeTime = progress >= 0.999f ? std::max(0.f, m_barPreviewTime - fillTime) : -1.f;

    setBarPreviewFill(progress, m_barFlashCycles, rate > 0.f, -1.f, completeTime);
}

// The rect gradient points are positioned relative to.
CCSize PointsLayer::getPreviewSize() {
    switch (m_page) {
        case EditorPage::Bar: return m_barFullSize * m_bar->getScale();
        case EditorPage::Wave: return m_wave->getContentSize();
        default: return ccp(30.5f, 30) * m_icon->getScale();
    }
}

CCPoint PointsLayer::getPreviewCenter() {
    switch (m_page) {
        case EditorPage::Bar: return m_bar->getPosition();
        case EditorPage::Wave: return m_wave->getPosition();
        default: return m_icon->getPosition();
    }
}

// Wave trail points stay on the line across the middle of the preview, and
// progress bar points on the line along the bar.
CCPoint PointsLayer::constrainPoint(CCPoint pos) {
    if (m_page == EditorPage::Icons) return pos;

    CCPoint center = getPreviewCenter();
    CCSize half = getPreviewSize() / 2.f;

    if (m_page == EditorPage::Bar)
        return ccp(
            std::max(center.x - half.width, std::min(pos.x, center.x + half.width)),
            center.y
        );

    return ccp(
        center.x,
        std::max(center.y - half.height, std::min(pos.y, center.y + half.height))
    );
}

int PointsLayer::getPointCount() {
    return m_points.size(); 
}

void PointsLayer::removeSelected() {
    if (!m_selectedPoint) selectLast();
    if (!m_selectedPoint) return;

    if (!m_pointsHidden) {
        addPoint(m_selectedPoint->getPosition());

        m_removingPoints.push_back(m_points.back());
        m_removingPoints.back()->setColor(m_selectedPoint->getColor());
        m_removingPoints.back()->setHidden(true, 0.3f);

        m_points.pop_back();
    }

    std::vector<ColorNode*> newPoints;

    for (ColorNode* point : m_points)
        if (point != m_selectedPoint)
            newPoints.push_back(point);

    m_points = newPoints;

    m_selectedPoint->removeFromParentAndCleanup(true);

    m_selectedPoint = nullptr;
}

void PointsLayer::moveSelected(const CCPoint& move) {
    CCPoint pos = m_selectedPoint->getPosition() + move;
    
    pos.x = std::max(0.f, std::min(pos.x, getContentSize().width));
    pos.y = std::max(0.f, std::min(pos.y, getContentSize().height));
    
    m_selectedPoint->setPosition(constrainPoint(pos));
    m_layer->pointMoved();
}

void PointsLayer::addPoint() {
    CCSize size = m_page != EditorPage::Icons ? getPreviewSize() : m_icon->getContentSize() * m_icon->getScale();
    CCPoint position = getPreviewCenter();

    std::vector<CCPoint> corners = {
        {position.x - size.width / 2, position.y + size.height / 2},
        {position.x + size.width / 2, position.y + size.height / 2},
        {position.x - size.width / 2, position.y - size.height / 2},
        {position.x + size.width / 2, position.y - size.height / 2}
    };

    // On the wave page new points go on the line: edge, middle, then between.
    if (m_page == EditorPage::Wave) {
        corners.clear();

        for (float y : { 0.5f, 0.f, 0.25f, 0.375f, 0.125f })
            corners.push_back({position.x, position.y + size.height * y});
    }

    // On the bar page they go along the bar: ends, middle, then between.
    if (m_page == EditorPage::Bar) {
        corners.clear();

        for (float x : { -0.5f, 0.5f, 0.f, -0.25f, 0.25f, -0.375f, 0.375f })
            corners.push_back({position.x + size.width * x, position.y});
    }

    CCPoint pos = {0, 0};
    
    for (const CCPoint& corner : corners) {
        bool taken = false;

        for (ColorNode* point : m_points) {
            if (
                static_cast<int>(point->getPosition().x) == static_cast<int>(corner.x)
                && static_cast<int>(point->getPosition().y) == static_cast<int>(corner.y)
            ) {
                taken = true;
                break;
            }
        }

        if (!taken) {
            pos = corner;
            break;
        }
    }

    if (pos == ccp(0, 0))
        pos = corners[std::rand() % corners.size()];

    addPoint(pos);
}

void PointsLayer::addPoint(const CCPoint& pos, bool invis) {
    ColorNode* node = ColorNode::create(invis, Mod::get()->getSettingValue<int64_t>("point-opacity"));

    node->setPosition(pos);
    node->setScale(Mod::get()->getSettingValue<double>("point-scale"));

    addChild(node);

    m_points.push_back(node);
}

ColorNode* PointsLayer::getNodeForPos(CCPoint pos) {
    pos = convertToNodeSpace(pos);

    ColorNode* ret = nullptr;
    float closest = getContentSize().width * 2;

    for (ColorNode* point : m_points) {
        float distance = ccpDistance(pos, point->getPosition());
        if (distance < closest && distance < point->getContentSize().width * 0.5f * point->getScale()) {
            closest = distance;
            ret = point;
        }
    }

    return ret;
}

void PointsLayer::selectFirst() {
    if (m_points.empty()) return;

    selectPoint(m_points.front());
}

void PointsLayer::selectLast() {
    if (m_points.empty()) return;
    
    selectPoint(m_points.back());
}

void PointsLayer::selectPoint(ColorNode* point) {
    if (!point) return;

    point->setSelected(true);

    if (m_selectedPoint && m_selectedPoint != point)
        m_selectedPoint->setSelected(false);

    m_selectedPoint = point;

    m_layer->pointSelected(point);
}

bool PointsLayer::ccTouchBegan(CCTouch* touch, CCEvent* event) { 
    if (m_isAnimating) return false;

    CCPoint pos = touch->getLocation();

    for (int i = 0; i < m_removingPoints.size(); i++)
        if (!m_removingPoints[i]->isAnimating()) {
            m_removingPoints[i]->removeFromParentAndCleanup(true);
            m_removingPoints.erase(m_removingPoints.begin() + i);
        }

    // Points sit on top of the bar, so they get the touch first.
    if (!getNodeForPos(pos) && dragBarPreview(touch, true))
        return true;

    if (ColorNode* point = getNodeForPos(pos)) {
        selectPoint(point);

        m_isMoving = true;
        m_moveOffset = point->getContentSize() * 0.5f * point->getScale() - point->convertToNodeSpace(pos)* point->getScale();

        if (Mod::get()->getSettingValue<bool>("hide-on-move")) {
            bool pointsHidden = m_pointsHidden;

            setPointsHidden(true, 0.3f);

            m_pointsHidden = pointsHidden;
        }

        return CCLayer::ccTouchBegan(touch, event);
    }

    return false;
}

void PointsLayer::ccTouchMoved(CCTouch* touch, CCEvent* event) {
    if (m_barDragging) {
        dragBarPreview(touch, false);
        return;
    }

    CCLayer::ccTouchMoved(touch, event);

    if (m_isMoving && m_selectedPoint) {
        CCPoint pos = convertToNodeSpace(touch->getLocation()) + m_moveOffset;

        pos.x = std::max(0.f, std::min(pos.x, getContentSize().width));
        pos.y = std::max(0.f, std::min(pos.y, getContentSize().height));

        m_selectedPoint->setPosition(constrainPoint(pos));

        m_layer->pointMoved();
    }
}

void PointsLayer::ccTouchEnded(CCTouch* touch, CCEvent* event) {
    if (m_barDragging) {
        m_barDragging = false;
        return;
    }

    m_isMoving = false;
    setPointsHidden(m_pointsHidden, 0.3f);
    m_layer->pointReleased();
}

std::vector<SimplePoint> PointsLayer::getPoints() {
    std::vector<SimplePoint> ret;

    for (ColorNode* point : m_points)
        ret.push_back({
            getRelativePos(point),
            point->getColor()
        });

    return ret;
}

IconType PointsLayer::getType() {
    return m_type;
}

void PointsLayer::updateHover(const CCPoint& pos) {
    if (ColorNode* point = getNodeForPos(pos)) {
        point->setHovered(true);

        if (m_hoveredPoint && m_hoveredPoint != point)
            m_hoveredPoint->setHovered(false);

        m_hoveredPoint = point;
    } else if (m_hoveredPoint) {
        m_hoveredPoint->setHovered(false);
        m_hoveredPoint = nullptr;
    }
}

void PointsLayer::updatePointOpacity(int value) {
    for (ColorNode* point : m_points)
        point->setOpacity(value);
}

void PointsLayer::updatePointScale(float value) {
    for (ColorNode* point : m_points)
        point->setScale(value);
}

void PointsLayer::updateGradient(GradientConfig config, ColorType colorType, bool force) {
    if (m_page == EditorPage::Bar) {
        // The fill may be part way through its preview loop, so everything is
        // measured against the full bar rather than what is filled right now.
        // Both of the bar's colours are shown, whichever is being edited.
        BarSprites bar;

        Utils::setupBar(bar, m_barFill, CCRect(0.f, 0.f, m_barFullSize.width, m_barFullSize.height), false);

        m_barGlow = bar.glow;
        m_barOverlay = bar.overlay;
        m_barOverlayGlow = bar.overlayGlow;
        m_barComplete = bar.complete;
        m_barCompleteGlow = bar.completeGlow;
        m_barEffects = Utils::getBarGlowEffects();
        m_barEditing = std::clamp(static_cast<int>(colorType) - 1, 0, 2);

        updateBarPreview(0.f);

        return;
    }

    if (m_page == EditorPage::Wave) {
        bool secondPlayer = m_layer->isSecondPlayer();

        CCSize size = m_wave->getContentSize();

        Utils::setIconColors(m_waveIcon, ColorType::Main, false, secondPlayer);

        // The same cross-section the game draws, on a straight trail.
        std::vector<ccV2F_C4B_T2F> vertices;

        CCPoint up = ccp(0.f, size.height / 2.f);
        CCPoint down = ccp(0.f, -size.height / 2.f);

        Utils::appendWaveSegment(
            Utils::computeWaveProfile(config, Utils::getWaveGlow(secondPlayer)),
            WaveEnd{ ccp(0.f, size.height / 2.f), up, down },
            WaveEnd{ ccp(size.width, size.height / 2.f), up, down },
            1.f,
            vertices
        );

        Utils::setDrawNodeVertices(m_waveDraw, vertices);

        return;
    }

    Utils::applyGradient(m_icon, config, colorType, false, m_layer->isSecondPlayer(), 1000);

    // Soft glow around the previewed icon. Robots and spiders are made of
    // many parts and have none, in the editor or in game.
    bool secondPlayer = m_layer->isSecondPlayer();
    bool hasGlow = m_type != IconType::Robot && m_type != IconType::Spider;

    GlowSettings glow = Utils::getGlow(GlowTarget::Icon, secondPlayer);

    if (!hasGlow)
        glow.enabled = false;

    Utils::updateSpriteGlow(
        m_icon->m_firstLayer,
        Utils::getSavedConfig(m_type, ColorType::Main, secondPlayer),
        m_type, 906, secondPlayer, false, 1000, glow
    );

    m_currentColor = colorType;

    updateCenter();
}

void PointsLayer::updateCenter() {
    m_icon->setContentSize(m_icon->m_firstLayer->getContentSize());
    m_icon->m_firstLayer->setPosition(
        m_icon->getContentSize() / 2.f
        - ccp(0, m_type == IconType::Ufo ? 8.f : 0.f)
    );

    // m_pointOffset = ccp(0.f, m_type == IconType::Ufo ? 10.f : 0.f);

    if (m_icon->m_robotSprite)
        m_icon->m_robotSprite->setPosition(m_icon->getContentSize() / 2.f);

    if (m_icon->m_spiderSprite)
        m_icon->m_spiderSprite->setPosition(m_icon->getContentSize() / 2.f);

    Utils::setIconColors(m_icon, m_currentColor, false, m_layer->isSecondPlayer());
}

ColorNode* PointsLayer::getSelectedPoint() {
    return m_selectedPoint;
}

SimplePlayer* PointsLayer::getIcon() {
    return m_icon;
}

void PointsLayer::setPlayerFrame(IconType type) {
    m_type = type;
    
    m_icon->updatePlayerFrame(
        Utils::getIconID(type, m_layer->isSecondPlayer()),
        type
    );

    // if (Loader::get()->isModLoaded("hiimjustin000.more_icons"))
    //     MoreIcons::updateSimplePlayer(m_icon, m_type, m_layer->isSecondPlayer());

    updateCenter();
}

void PointsLayer::setPointsHidden(bool hidden, float time) {
    if (m_points.empty()) return;

    float delay = time / m_points.size();
    
    for (int i = 0; i < m_points.size(); i++)
        m_points[i]->setHidden(hidden, delay * (i + 1) + (time > 0.f ? 0.1f : 0.f));

    m_pointsHidden = hidden;
}

CCPoint PointsLayer::getRelativePos(ColorNode* point) {
    if (!point) return {0, 0};

    CCSize iconSize = getPreviewSize();
    CCPoint realPos = point->getPosition() + m_pointOffset - (getPreviewCenter() - iconSize * (m_page != EditorPage::Icons ? ccp(0.5f, 0.5f) : m_icon->getAnchorPoint()));

    return {
        realPos.x / iconSize.width,
        realPos.y / iconSize.height
    };
}

void PointsLayer::loadPoints(GradientConfig config, bool animate) {
    if (m_page == EditorPage::Wave)
        updateWaveLayout();

    updateCenter();

    m_currentConfig = config;
    m_isAnimating = true;

    if (!animate || m_pointsHidden) return onAnimationEnded();

    // m_icon->runAction(CCFadeTo::create(0.05f, 0));

    CCSize iconSize = getPreviewSize();
    CCPoint bottomLeft = getPreviewCenter() - iconSize / 2.f;
    
    std::unordered_set<SimplePoint> replacedPoints;
    std::unordered_set<ColorNode*> movedPoints;

    std::vector<ColorNode*> points = m_selectedPoint
        ? std::vector<ColorNode*>{ m_selectedPoint }
        : std::vector<ColorNode*>{};

    for (ColorNode* point : m_points)
        if (point != m_selectedPoint)
            points.push_back(point);

    for (int i = 0; i < m_currentConfig.points.size(); i++) {
        if (i == points.size()) break;

        replacedPoints.insert(m_currentConfig.points[i]);

        CCPoint pos = bottomLeft + m_currentConfig.points[i].pos * iconSize;
        ccColor3B color = m_currentConfig.points[i].color;

        points[i]->setColor(color, 0.2f);
        points[i]->runAction(CCEaseSineOut::create(CCMoveTo::create(0.1f, pos)));

        movedPoints.insert(points[i]);
    }

    bool addedSelected = false;

    for (const SimplePoint& point : m_currentConfig.points) {
        if (replacedPoints.contains(point)) continue;
        
        addPoint(bottomLeft + point.pos * iconSize, true);

        if (!m_selectedPoint && !config.points.empty() && !addedSelected) {
            m_points.back()->setSelected(true);

            m_selectedPoint = m_points.back();
            addedSelected = true;
        }

        m_points.back()->setColor(point.color);
        m_points.back()->setHidden(false, 0.1f);

        movedPoints.insert(m_points.back());
    }

    for (ColorNode* point : m_points)
        if (!movedPoints.contains(point))
            point->setHidden(true, 0.1f);

    runAction(CCSequence::create(
        CCDelayTime::create(0.1f),
        CCCallFunc::create(this, callfunc_selector(PointsLayer::onAnimationEnded)),
        nullptr 
    ));

}

void PointsLayer::onAnimationEnded() {
    // m_icon->runAction(CCFadeTo::create(0.05f, 255));

    CCSize iconSize = getPreviewSize();
    CCPoint bottomLeft = getPreviewCenter() - iconSize / 2.f;
    CCPoint selectPos = {0, 0};

    for (ColorNode* point : m_points)
        if (point->isSelected()) {
            selectPos = point->getPosition();
            break;
        }
        
    for (ColorNode* point : m_points)
        point->removeFromParentAndCleanup(true);

    m_points.clear();

    m_isAnimating = false;
    m_selectedPoint = nullptr;
    m_hoveredPoint = nullptr;

    ColorNode* realSelectedPoint = nullptr;

    for (const SimplePoint& point : m_currentConfig.points) {
        CCPoint pos = bottomLeft + point.pos * iconSize;

        addPoint(pos);
        
        m_points.back()->setColor(point.color);

        if (std::abs(pos.x - selectPos.x) < 0.001f && std::abs(pos.y - selectPos.y) < 0.001f) {
            m_points.back()->setSelected(true);

            realSelectedPoint = m_points.back();
            selectPos = ccp(0, 0);
        }
    }

    selectPoint(realSelectedPoint);
}
