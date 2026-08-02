#include "BM176LookAndFeel.h"
#include "BM176Colours.h"

namespace bm176
{
    BM176LookAndFeel::BM176LookAndFeel()
    {
        setColour(juce::ResizableWindow::backgroundColourId, panelBase);
    }

    juce::Typeface::Ptr BM176LookAndFeel::getTypefaceForFont(const juce::Font& font)
    {
        return juce::Typeface::createSystemTypefaceFor(font);
    }

    juce::Font BM176LookAndFeel::getLabelFont(juce::Label&)
    {
        return juce::Font(juce::FontOptions().withHeight(14.0f));
    }
}
