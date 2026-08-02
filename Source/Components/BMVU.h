#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../GUI/BM176Geometry.h"

namespace bm176
{
    class BMVU : public juce::Component, private juce::Timer
    {
    public:
        BMVU();
        ~BMVU() override;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void setTargetDB(float dB);
        void setMode(bool grMode);

        void timerCallback() override;

    private:
        float targetDB  = 0.0f;
        float displayDB = 0.0f;
        float velocity  = 0.0f;
        bool  grMode    = false;

        void drawBezel(juce::Graphics& g);
        void drawGlassRecess(juce::Graphics& g);
        void drawFaceBg(juce::Graphics& g);
        void drawRedArc(juce::Graphics& g);
        void drawDbScale(juce::Graphics& g);
        void drawPctScale(juce::Graphics& g);
        void drawFaceDecor(juce::Graphics& g);
        void drawNeedle(juce::Graphics& g);
        void drawGlare(juce::Graphics& g);

        float pctToAngle(float pct) const;
        juce::Point<float> pointOnArc(float radius, float angleDeg) const;

        float glassX = 0, glassY = 0, glassW = VU_GLASS_W, glassH = VU_GLASS_H;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BMVU)
    };
}
