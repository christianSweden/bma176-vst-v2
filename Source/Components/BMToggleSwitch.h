#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace bm176
{
    class BMToggleSwitch : public juce::Component
    {
    public:
        BMToggleSwitch();

        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& e) override;

        void setState(bool up);
        bool getState() const;

    private:
        bool state = true;
        float animLerp = 1.0f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BMToggleSwitch)
    };
}
