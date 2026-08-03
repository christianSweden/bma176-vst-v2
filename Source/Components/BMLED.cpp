#include "BMLED.h"
#include "../GUI/BM176Colours.h"

namespace bm176
{
    BMLED::BMLED()
    {
        setRepaintsOnMouseActivity(false);
    }

    void BMLED::setState(bool on) { if (state != on) { state = on; repaint(); } }
    bool BMLED::getState() const { return state; }

    void BMLED::paint(juce::Graphics& g)
    {
        const float cx = getWidth() * 0.5f;
        const float cy = getHeight() * 0.5f;
        const float R = 15.0f;
        const float dim = state ? 1.0f : 0.20f;

        juce::ColourGradient bloom(
            lampOnCore.withAlpha(0.22f * dim), cx, cy,
            lampOnCore.withAlpha(0.0f), cx + 35.0f, cy + 35.0f, true);
        g.setGradientFill(bloom);
        g.fillEllipse(cx - 35.0f, cy - 35.0f, 70.0f, 70.0f);

        juce::ColourGradient body(
            lampOnCore.withAlpha(dim), cx - R * 0.35f, cy - R * 0.35f,
            lampOnRim.withAlpha(dim), cx + R, cy + R, true);
        g.setGradientFill(body);
        g.fillEllipse(cx - R, cy - R, R * 2.0f, R * 2.0f);

        juce::Path crescent;
        crescent.addCentredArc(cx - R * 0.15f, cy - R * 0.15f, R * 0.75f, R * 0.75f, 0.0f,
            juce::degreesToRadians(-130.0f), juce::degreesToRadians(-40.0f), true);
        g.setColour(juce::Colours::white.withAlpha(0.35f * dim));
        g.strokePath(crescent, juce::PathStrokeType(2.0f));

        g.setColour(chromeTop);
        g.drawEllipse(cx - R, cy - R, R * 2.0f, R * 2.0f, 3.0f);
        g.setColour(chromeMid);
        g.drawEllipse(cx - R + 1.5f, cy - R + 1.5f, (R - 1.5f) * 2.0f, (R - 1.5f) * 2.0f, 1.0f);
    }
}
