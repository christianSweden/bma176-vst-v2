#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

namespace bm176
{
    inline constexpr int   DESIGN_WIDTH          = 1920;
    inline constexpr int   DESIGN_HEIGHT         = 354;
    inline constexpr float DESIGN_ASPECT         = 1920.0f / 354.0f;

    inline constexpr int   DEFAULT_WINDOW_W      = 1440;
    inline constexpr int   DEFAULT_WINDOW_H      = 266;

    inline constexpr float KNOB_BIG_R            = 79.0f;
    inline constexpr float KNOB_SMALL_R          = 41.0f;

    inline constexpr float SCREW_R               = 13.0f;
    inline constexpr float JACK_OUTER_R          = 23.0f;
    inline constexpr float JACK_INNER_R          = 14.0f;
    inline constexpr float JACK_BORE_R           = 7.5f;

    inline constexpr float SWITCH_NUT_ACROSS     = 17.0f;
    inline constexpr float SWITCH_NUT_HEIGHT     = 20.0f;

    inline constexpr float VU_BEZEL_X            = 758.0f;
    inline constexpr float VU_BEZEL_Y            = 20.0f;
    inline constexpr float VU_BEZEL_W            = 438.0f;
    inline constexpr float VU_BEZEL_H            = 220.0f;
    inline constexpr float VU_BEZEL_WIDTH        = 16.0f;
    inline constexpr float VU_GLASS_X            = 774.0f;
    inline constexpr float VU_GLASS_Y            = 36.0f;
    inline constexpr float VU_GLASS_W            = 406.0f;
    inline constexpr float VU_GLASS_H            = 188.0f;

    inline constexpr float PANEL_TEXT_SIZE       = 16.0f;
    inline constexpr float PANEL_TEXT_SMALL      = 13.0f;
    inline constexpr float PANEL_TEXT_LARGE      = 18.0f;

    inline juce::AffineTransform getScaleTransform(int targetWidth, int targetHeight)
    {
        const float sx = static_cast<float>(targetWidth)  / static_cast<float>(DESIGN_WIDTH);
        const float sy = static_cast<float>(targetHeight) / static_cast<float>(DESIGN_HEIGHT);
        const float s = juce::jmin(sx, sy);
        return juce::AffineTransform::scale(s);
    }

    inline juce::Rectangle<float> getDesignBounds(int targetWidth, int targetHeight)
    {
        const float sx = static_cast<float>(targetWidth)  / static_cast<float>(DESIGN_WIDTH);
        const float sy = static_cast<float>(targetHeight) / static_cast<float>(DESIGN_HEIGHT);
        const float s = juce::jmin(sx, sy);
        const float dw = static_cast<float>(DESIGN_WIDTH)  * s;
        const float dh = static_cast<float>(DESIGN_HEIGHT) * s;
        const float ox = (static_cast<float>(targetWidth)  - dw) * 0.5f;
        const float oy = (static_cast<float>(targetHeight) - dh) * 0.5f;
        return juce::Rectangle<float>(ox, oy, dw, dh);
    }
}
