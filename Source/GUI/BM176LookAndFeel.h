#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace bm176
{
    class BM176LookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        BM176LookAndFeel();
        ~BM176LookAndFeel() override = default;

        juce::Typeface::Ptr getTypefaceForFont(const juce::Font&) override;

        juce::Font getLabelFont(juce::Label&) override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BM176LookAndFeel)
    };
}
