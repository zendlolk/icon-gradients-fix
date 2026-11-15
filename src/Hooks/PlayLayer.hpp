#include "Includes.hpp"

#include <Geode/modify/PlayLayer.hpp>

class $modify(ProPlayLayer, PlayLayer) {

    struct Fields {
        bool m_barGradientApplied = false;
    };

    void updateBarGradient();

    void postUpdate(float);

};