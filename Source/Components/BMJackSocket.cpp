#include "BMJackSocket.h"
#include "../GUI/BM176Colours.h"

namespace bm176
{
    BMJackSocket::BMJackSocket()
    {
        setRepaintsOnMouseActivity(false);
    }

    void BMJackSocket::paint(juce::Graphics& g)
    {
        const float cx = getWidth() * 0.5f;
        const float cy = getHeight() * 0.5f;
        const float outR = 23.0f;
        const float inR = 14.0f;
        const float boreR = 7.5f;

        juce::ColourGradient outerGrad(jackOuterTop, cx - outR, cy - outR,
                                       jackOuterBottom, cx + outR, cy + outR, false);
        g.setGradientFill(outerGrad);
        g.fillEllipse(cx - outR, cy - outR, outR * 2.0f, outR * 2.0f);

        juce::ColourGradient innerGrad(jackInnerTop, cx - inR, cy - inR,
                                       jackInnerBottom, cx + inR, cy + inR, false);
        g.setGradientFill(innerGrad);
        g.fillEllipse(cx - inR, cy - inR, inR * 2.0f, inR * 2.0f);

        g.setColour(juce::Colours::black);
        g.fillEllipse(cx - boreR, cy - boreR, boreR * 2.0f, boreR * 2.0f);
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.drawEllipse(cx - boreR, cy - boreR, boreR * 2.0f, boreR * 2.0f, 2.0f);

        juce::Path spec;
        spec.addCentredArc(cx - inR * 0.25f, cy - inR * 0.25f, inR * 0.85f, inR * 0.85f, 0.0f,
            juce::degreesToRadians(200.0f), juce::degreesToRadians(300.0f), true);
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.strokePath(spec, juce::PathStrokeType(1.5f));
    }
}
