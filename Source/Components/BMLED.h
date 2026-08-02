#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace bm176
{
    class BMLED : public juce::Component
    {
    public:
        BMLED();

        void paint(juce::Graphics& g) override;
        void setState(bool on);
        bool getState() const;

    private:
        bool state = false;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BMLED)
    };
}
