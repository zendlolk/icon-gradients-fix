#include "Includes.hpp"

#include <Geode/modify/PlayLayer.hpp>

class $modify(ProPlayLayer, PlayLayer) {

    struct Fields {
        bool m_barGradientApplied = false;
        Ref<CCSprite> m_barGlow = nullptr;
    };

    void updateBarGradient();

    void postUpdate(float);

};