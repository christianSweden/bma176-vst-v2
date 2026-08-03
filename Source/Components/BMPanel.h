#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace bm176
{
    class BMPanel : public juce::Component
    {
    public:
        BMPanel();
        void paint(juce::Graphics& g) override;
        void resized() override;

    private:
        juce::Image brushedTexture;
        bool textureBuilt = false;

        void buildTexture();

        void drawSectionLabel(juce::Graphics& g, float cx, float cy, float sizeH,
                             const juce::String& text);
        void drawKnobScaleNumbers(juce::Graphics& g, float cx, float cy, float R,
                                  const juce::StringArray& labels, const juce::StringArray& angles,
                                  const juce::StringArray& xo = {}, const juce::StringArray& yo = {});
        void drawKnobDots(juce::Graphics& g, float cx, float cy, float R,
                         int count, float minAngle, float maxAngle);
        void drawKnobDotsAt(juce::Graphics& g, float cx, float cy, float R,
                           const std::vector<float>& angles, float dotSize = -1.0f,
                           float ringFactor = 1.26f);
        void drawEndWords(juce::Graphics& g, float cx, float cy, float R,
                         const juce::String& leftWord, const juce::String& rightWord);
        void drawDiscreteLabels(juce::Graphics& g, float cx, float cy, float R,
                               const juce::StringArray& labels, const juce::StringArray& angles,
                               bool drawDots = true,
                               const juce::StringArray& yOffsets = {});
        void drawScrew(juce::Graphics& g, float cx, float cy, float r, float rotationDeg);
        void drawWordmark(juce::Graphics& g, float cx, float cy, float capHeight);
        void drawPanelText(juce::Graphics& g, float cx, float y, float size,
                          const juce::String& text, float trackingEm);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BMPanel)
    };
}
