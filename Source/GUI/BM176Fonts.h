#pragma once

#include <juce_graphics/juce_graphics.h>

namespace bm176
{
    inline juce::String hwString(int value)
    {
        if (value >= 10)
        {
            const int tens = value / 10;
            const int ones = value % 10;
            juce::String s;
            if (tens == 1) s << "I";
            else s << tens;
            s << ones;
            return s;
        }
        return juce::String(value);
    }

    inline juce::Font resolveCondensedFont(float height)
    {
        static const juce::StringArray candidates = {
            "Roboto Condensed", "Arial Narrow",
            "Helvetica Neue Condensed", "DIN Condensed"
        };
        for (auto& name : candidates)
        {
            juce::Font f(juce::FontOptions().withName(name).withHeight(height).withStyle("bold"));
            if (f.getTypefaceName().containsIgnoreCase(name.substring(0, 6)))
                return f;
        }
        return juce::Font(juce::FontOptions().withHeight(height).withStyle("bold"));
    }

    inline void drawTrackedText(juce::Graphics& g, const juce::String& text,
                                juce::Font font, juce::Colour colour,
                                float cx, float cy, float trackingEm,
                                juce::Colour shadowCol = juce::Colours::black.withAlpha(0.55f))
    {
        juce::GlyphArrangement ga;
        ga.addLineOfText(font, text, 0.0f, 0.0f);
        float offset = 0.0f;
        for (int i = 0, n = ga.getNumGlyphs(); i < n; ++i)
        {
            ga.getGlyph(i).moveBy(offset, 0.0f);
            offset += trackingEm * font.getHeight();
        }
        juce::Rectangle<float> tb = ga.getBoundingBox(0, -1, true);
        ga.moveRangeOfGlyphs(0, -1, cx - tb.getCentreX(), cy - tb.getCentreY());

        juce::Path sp, mp;
        ga.createPath(sp);
        {
            juce::Graphics::ScopedSaveState ss(g);
            g.addTransform(juce::AffineTransform::translation(0.0f, 1.0f));
            g.setColour(shadowCol);
            g.fillPath(sp);
        }
        ga.createPath(mp);
        g.setColour(colour);
        g.fillPath(mp);
    }
}
