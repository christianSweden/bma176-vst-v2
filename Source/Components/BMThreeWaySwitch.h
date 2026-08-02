#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace bm176
{
    class BMThreeWaySwitch : public juce::Component
    {
    public:
        BMThreeWaySwitch();

        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& e) override;

        void setPosition(int pos);
        int getPosition() const;

        void setLabels(const juce::String& title,
                       const juce::String& leftLabel,
                       const juce::String& centerLabel,
                       const juce::String& rightLabel);

    private:
        int position = 1;
        juce::String title;
        juce::String leftText;
        juce::String centerText;
        juce::String rightText;

        void drawBushing(juce::Graphics& g, float cx, float by);
        void drawLever(juce::Graphics& g, float cx, float by);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BMThreeWaySwitch)
    };
}
