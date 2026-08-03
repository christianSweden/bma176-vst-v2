#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace bm176
{
    class BMToggleSwitch : public juce::Component, private juce::Timer
    {
    public:
        BMToggleSwitch();
        ~BMToggleSwitch() override;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& e) override;

        void setState(bool up);
        bool getState() const;

        void timerCallback() override;

    private:
        bool state = true;
        float animT = 1.0f;
        float animTarget = 1.0f;
        float animVelocity = 0.0f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BMToggleSwitch)
    };
}
