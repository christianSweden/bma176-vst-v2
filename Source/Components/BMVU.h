#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace bm176
{
    class BMVU : public juce::Component
    {
    public:
        BMVU();

        void paint(juce::Graphics& g) override;
        void resized() override;

        void setValue(float dB);
        void setMode(bool grMode);

    private:
        float needleDB = 0.0f;
        bool  grMode   = false;

        void drawBezel(juce::Graphics& g);
        void drawGlassRecess(juce::Graphics& g);
        void drawFaceBg(juce::Graphics& g);
        void drawRedArc(juce::Graphics& g);
        void drawDbScale(juce::Graphics& g);
        void drawPctScale(juce::Graphics& g);
        void drawNeedle(juce::Graphics& g);
        void drawFaceDecor(juce::Graphics& g);
        void drawGlare(juce::Graphics& g);

        float pctToAngle(float pct) const;
        juce::Point<float> pointOnArc(float radius, float angleDeg) const;

        float glassX, glassY, glassW, glassH;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BMVU)
    };
}
