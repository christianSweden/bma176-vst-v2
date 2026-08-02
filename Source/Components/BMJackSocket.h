#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace bm176
{
    class BMJackSocket : public juce::Component
    {
    public:
        BMJackSocket();
        void paint(juce::Graphics& g) override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BMJackSocket)
    };
}
