#pragma once

#include "Includes.hpp"

#include <array>
#include <vector>

#include <Geode/modify/HardStreak.hpp>
#include <Geode/modify/CCDrawNode.hpp>

class $modify(ProHardStreak, HardStreak) {

    bool captureStroke(float, float, std::vector<std::array<CCPoint, 4>>&);

    void updateStroke(float);

};

class $modify(ProDrawNode, CCDrawNode) {

    bool drawPolygon(CCPoint*, unsigned int, const ccColor4F&, float, const ccColor4F&, BorderAlignment);

};
