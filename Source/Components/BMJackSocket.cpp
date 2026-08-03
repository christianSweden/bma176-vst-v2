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
        const float inR  = 14.0f;
        const float boreR = 8.0f;

        juce::Path shadowPath;
        shadowPath.addEllipse(juce::Rectangle<float>(cx - outR, cy - outR, outR * 2.0f, outR * 2.0f));
        juce::DropShadow baseShadow(juce::Colours::black.withAlpha(0.45f), 8, juce::Point<int>(0, 3));
        baseShadow.drawForPath(g, shadowPath);

        juce::ColourGradient outerGrad(jackOuterTop, cx - outR * 0.3f, cy - outR * 0.3f,
                                       jackOuterBottom, cx + outR * 0.3f, cy + outR * 0.3f, false);
        g.setGradientFill(outerGrad);
        g.fillEllipse(cx - outR, cy - outR, outR * 2.0f, outR * 2.0f);

        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.drawEllipse(cx - outR, cy - outR, outR * 2.0f, outR * 2.0f, 1.2f);

        juce::ColourGradient innerGrad(jackInnerTop, cx - inR * 0.2f, cy - inR * 0.2f,
                                       jackInnerBottom, cx + inR * 0.2f, cy + inR * 0.2f, false);
        g.setGradientFill(innerGrad);
        g.fillEllipse(cx - inR, cy - inR, inR * 2.0f, inR * 2.0f);

        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.drawEllipse(cx - inR, cy - inR, inR * 2.0f, inR * 2.0f, 1.0f);

        juce::ColourGradient boreGrad(juce::Colours::black, cx - boreR, cy - boreR,
                                      juce::Colours::black.darker(0.2f), cx + boreR, cy + boreR, false);
        g.setGradientFill(boreGrad);
        g.fillEllipse(cx - boreR, cy - boreR, boreR * 2.0f, boreR * 2.0f);

        juce::ColourGradient innerShadow(
            juce::Colours::black.withAlpha(0.6f), cx - boreR, cy - boreR,
            juce::Colours::black.withAlpha(0.0f), cx, cy, true);
        g.setGradientFill(innerShadow);
        g.fillEllipse(cx - boreR, cy - boreR, boreR * 2.0f, boreR * 2.0f);

        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawEllipse(cx - boreR, cy - boreR, boreR * 2.0f, boreR * 2.0f, 1.0f);

        juce::Path spec;
        spec.addCentredArc(cx - inR * 0.2f, cy - inR * 0.2f, inR * 0.75f, inR * 0.75f, 0.0f,
            juce::degreesToRadians(210.0f), juce::degreesToRadians(310.0f), true);
        g.setColour(juce::Colours::white.withAlpha(0.15f));
        g.strokePath(spec, juce::PathStrokeType(1.2f));

        g.setColour(juce::Colours::white.withAlpha(0.06f));
        g.drawLine(cx - outR * 0.4f, cy - outR * 0.6f, cx + outR * 0.5f, cy - outR * 0.6f, 0.8f);
    }
}
