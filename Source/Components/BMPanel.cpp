#include "BMPanel.h"
#include "../GUI/BM176Colours.h"
#include "../GUI/BM176Geometry.h"
#include "../GUI/BM176Fonts.h"

namespace bm176
{
    BMPanel::BMPanel() {}

    void BMPanel::resized() { textureBuilt = false; }

    void BMPanel::buildTexture()
    {
        const int w = getWidth();
        const int h = getHeight();
        if (w <= 0 || h <= 0) return;

        brushedTexture = juce::Image(juce::Image::ARGB, w, h, true);
        juce::Graphics tg(brushedTexture);
        juce::Random rng(0x42);
        for (int y = 0; y < h; ++y)
        {
            const float a = rng.nextFloat() * 0.03f + 0.02f;
            tg.setColour(juce::Colour(1.0f, 1.0f, 1.0f).withAlpha(a));
            tg.drawLine(0.0f, static_cast<float>(y), static_cast<float>(w), static_cast<float>(y), 1.0f);
        }
        textureBuilt = true;
    }

    void BMPanel::paint(juce::Graphics& g)
    {
        const float w = static_cast<float>(getWidth());
        const float h = static_cast<float>(getHeight());

        if (!textureBuilt || brushedTexture.getWidth() != getWidth() || brushedTexture.getHeight() != getHeight())
            buildTexture();

        juce::ColourGradient panelGrad(panelBase, 0.0f, 0.0f, panelDeep, 0.0f, h, false);
        g.setGradientFill(panelGrad);
        g.fillAll();

        g.setOpacity(1.0f);
        g.drawImageAt(brushedTexture, 0, 0, false);

        g.setColour(panelEdgeHi.withAlpha(0.25f));
        g.drawLine(0.0f, 0.0f, w, 0.0f, 2.0f);

        juce::ColourGradient vignette(
            juce::Colours::black.withAlpha(0.0f), w * 0.5f, h * 0.5f,
            juce::Colours::black.withAlpha(0.12f), w, h, true);
        g.setGradientFill(vignette);
        g.fillAll();

        constexpr float inset = 11.0f;
        constexpr float cr = 6.0f;
        juce::Path borderPath;
        borderPath.addRoundedRectangle(inset, inset, w - inset * 2.0f, h - inset * 2.0f, cr);

        juce::PathStrokeType strokeType(1.5f);
        juce::Path outerBorder, innerBorder;
        strokeType.createStrokedPath(outerBorder, borderPath, juce::AffineTransform::translation(0.0f, 1.0f));
        g.setColour(panelGrooveInner.withAlpha(0.6f));
        g.fillPath(outerBorder);
        strokeType.createStrokedPath(innerBorder, borderPath, juce::AffineTransform::translation(0.0f, -1.0f));
        g.setColour(panelGrooveOuter.withAlpha(0.45f));
        g.fillPath(innerBorder);

        drawScrew(g,  34.0f,  22.0f, SCREW_R, 12.0f);
        drawScrew(g, 1886.0f, 22.0f, SCREW_R, -31.0f);
        drawScrew(g,  34.0f, 332.0f, SCREW_R, 47.0f);
        drawScrew(g, 1886.0f, 332.0f, SCREW_R, 8.0f);

        drawSectionLabel(g,  90.0f,  41.0f, 13.0f, "INPUT");
        drawSectionLabel(g,  90.0f, 135.0f, 13.0f, "HI Z INPUT");
        drawSectionLabel(g,  90.0f, 218.0f, 13.0f, "OUTPUT");

        drawSectionLabel(g, 196.0f,  22.0f, 16.0f, "COMP. RATIO");
        drawSectionLabel(g, 196.0f, 194.0f, 16.0f, "SIDECHAIN");
        drawSectionLabel(g, 376.0f,  44.0f, 18.0f, "INPUT");
        drawSectionLabel(g, 567.0f,  25.0f, 16.0f, "METER");
        drawSectionLabel(g, 567.0f, 196.0f, 16.0f, "VERNIER");
        drawSectionLabel(g, 679.0f, 100.0f, 16.0f, "THRESHOLD");
        drawSectionLabel(g, 1243.0f, 118.0f, 16.0f, "INTERSTAGE");
        drawSectionLabel(g, 1243.0f, 140.0f, 12.0f, "IN");
        drawSectionLabel(g, 1243.0f, 248.0f, 12.0f, "OUT");
        drawSectionLabel(g, 1243.0f, 256.0f, 13.0f, "COMP");
        drawSectionLabel(g, 1364.0f,  24.0f, 16.0f, "ATTACK");
        drawSectionLabel(g, 1364.0f, 207.0f, 16.0f, "VERNIER");
        drawSectionLabel(g, 1555.0f,  44.0f, 18.0f, "OUTPUT");
        drawSectionLabel(g, 1734.0f,  24.0f, 16.0f, "RELEASE");
        drawSectionLabel(g, 1848.0f,  26.0f, 13.0f, "IN");
        drawSectionLabel(g, 1848.0f, 134.0f, 13.0f, "BYPASS");
        drawSectionLabel(g, 1848.0f, 196.0f, 13.0f, "ON");

        juce::StringArray inputLabels;
        juce::StringArray inputAngles;
        for (int v = 40; v >= 0; v -= 4) {
            inputLabels.add(hwString(v));
            inputAngles.add(juce::String((static_cast<float>(v) - 20.0f) * 7.5f));
        }
        drawKnobScaleNumbers(g, 376.0f, 171.0f, 79.0f, inputLabels, inputAngles,
            {}, {}, 15.0f, 1.324f);
        drawKnobDots(g, 376.0f, 171.0f, 79.0f, 21, -150.0f, 150.0f, 1.134f);

        juce::StringArray outputLabels;
        juce::StringArray outputAngles;
        for (int v = 40; v >= 0; v -= 4) {
            outputLabels.add(hwString(v));
            outputAngles.add(juce::String((static_cast<float>(v) - 20.0f) * 7.5f));
        }
        drawKnobScaleNumbers(g, 1555.0f, 171.0f, 79.0f, outputLabels, outputAngles,
            {}, {}, 15.0f, 1.324f);
        drawKnobDots(g, 1555.0f, 171.0f, 79.0f, 21, -150.0f, 150.0f, 1.134f);

        juce::StringArray smallNums{"2", "4", "6", "8"};
        juce::StringArray smallAngles{"-90", "-45", "45", "90"};
        drawKnobScaleNumbers(g, 679.0f, 166.0f, 41.0f, smallNums, smallAngles,
            juce::StringArray{"0", "2", "-2", "0"},
            juce::StringArray{"0", "-3", "-3", "0"}, 14.0f);
        drawKnobDotsAt(g, 679.0f, 166.0f, 41.0f,
            std::vector<float>{-150.0f, -120.0f, -90.0f, -67.5f, -45.0f, 0.0f, 45.0f, 67.5f, 90.0f, 120.0f, 150.0f},
            3.25f, 1.236f);

        drawKnobScaleNumbers(g, 1364.0f, 88.0f, 41.0f, smallNums, smallAngles,
            juce::StringArray{"0", "2", "-2", "0"},
            juce::StringArray{"0", "-3", "-3", "0"}, 14.0f);
        drawKnobDotsAt(g, 1364.0f, 88.0f, 41.0f,
            std::vector<float>{-150.0f, -120.0f, -90.0f, -67.5f, -45.0f, 0.0f, 45.0f, 67.5f, 90.0f, 120.0f, 150.0f},
            3.25f, 1.236f);
        drawEndWords(g, 1364.0f, 88.0f, 41.0f, "OFF", "FAST");

        drawKnobScaleNumbers(g, 1734.0f, 88.0f, 41.0f, smallNums, smallAngles,
            juce::StringArray{"0", "2", "-2", "0"},
            juce::StringArray{"0", "-3", "-3", "0"}, 14.0f);
        drawKnobDotsAt(g, 1734.0f, 88.0f, 41.0f,
            std::vector<float>{-150.0f, -120.0f, -90.0f, -67.5f, -45.0f, 0.0f, 45.0f, 67.5f, 90.0f, 120.0f, 150.0f},
            3.25f, 1.236f);
        drawEndWords(g, 1734.0f, 88.0f, 41.0f, "SLOW", "FAST");

        drawDiscreteLabels(g, 196.0f, 103.0f, 41.0f,
            juce::StringArray{"I.5:I", "2:I", "4:I", "8:I", "I2:I"},
            juce::StringArray{"-60", "-30", "0", "30", "60"}, false,
            juce::StringArray{"5", "-5", "-10", "-5", "5"});
        drawDiscreteLabels(g, 196.0f, 264.0f, 41.0f,
            juce::StringArray{"OFF", "45", "80", "I20", "I50", "200"},
            juce::StringArray{"-75", "-45", "-15", "15", "45", "75"}, false,
            juce::StringArray{"10", "0", "-10", "-10", "0", "10"});
        drawDiscreteLabels(g, 567.0f, 98.0f, 41.0f,
            juce::StringArray{"IN", "GR", "OUT"},
            juce::StringArray{"-45", "0", "45"}, false,
            juce::StringArray{"-10", "-10", "-10"});

        drawWordmark(g, 977.0f, 296.0f, 40.0f);
        drawPanelText(g, 977.0f, 336.0f, 19.0f, "LIMITING  AMPLIFIER   BM 176", 0.22f);
    }

    void BMPanel::drawSectionLabel(juce::Graphics& g, float cx, float cy, float sizeH,
                                    const juce::String& text)
    {
        juce::Font font = resolveCondensedFont(sizeH);
        bool needsScaling = !font.getTypefaceName().containsIgnoreCase("condensed")
                         && !font.getTypefaceName().containsIgnoreCase("narrow");
        {
            juce::Graphics::ScopedSaveState ss(g);
            if (needsScaling)
                g.addTransform(juce::AffineTransform::scale(0.88f, 1.0f).translated(cx * 0.136f, 0.0f));
            drawTrackedText(g, text.toUpperCase(), font, textMain.withAlpha(0.92f),
                           cx, cy, 0.12f, textShadow.withAlpha(0.55f));
        }
    }

    void BMPanel::drawKnobScaleNumbers(juce::Graphics& g, float cx, float cy, float R,
                                       const juce::StringArray& labels,
                                       const juce::StringArray& angles,
                                       const juce::StringArray& xo,
                                       const juce::StringArray& yo,
                                       float fontSize,
                                       float ringFactor)
    {
        const float ringR = ringFactor * R;
        g.setFont(juce::Font(juce::FontOptions().withHeight(fontSize)));
        g.setColour(dotColour.withAlpha(0.85f));
        for (int i = 0; i < labels.size() && i < angles.size(); ++i)
        {
            const float aDeg = angles[i].getFloatValue();
            const float aRad = juce::degreesToRadians(aDeg);
            float lx = cx + ringR * std::sin(aRad);
            float ly = cy - ringR * std::cos(aRad);
            if (i < xo.size()) lx += xo[i].getFloatValue();
            if (i < yo.size()) ly += yo[i].getFloatValue();
            g.drawText(labels[i], juce::Rectangle<float>(lx - 16.0f, ly - 10.0f, 32.0f, 20.0f),
                       juce::Justification::centred);
        }
    }

    void BMPanel::drawKnobDotsAt(juce::Graphics& g, float cx, float cy, float R,
                                const std::vector<float>& angles, float dotSize,
                                float ringFactor)
    {
        const float ringR = ringFactor * R;
        if (dotSize < 0.0f)
            dotSize = (R > 70.0f) ? 3.0f : 2.25f;
        g.setColour(dotColour.withAlpha(0.85f));
        for (float aDeg : angles)
        {
            const float aRad = juce::degreesToRadians(aDeg);
            const float dx = cx + ringR * std::sin(aRad);
            const float dy = cy - ringR * std::cos(aRad);
            g.fillEllipse(dx - dotSize, dy - dotSize, dotSize * 2.0f, dotSize * 2.0f);
        }
    }

    void BMPanel::drawKnobDots(juce::Graphics& g, float cx, float cy, float R,
                               int count, float minAngle, float maxAngle, float ringFactor)
    {
        const float ringR = ringFactor * R;
        const float dotSize = (R > 70.0f) ? 3.0f : 2.25f;
        const int n = count;
        const float range = maxAngle - minAngle;
        g.setColour(dotColour.withAlpha(0.85f));
        for (int i = 0; i < n; ++i)
        {
            const float t = (n > 1) ? static_cast<float>(i) / static_cast<float>(n - 1) : 0.0f;
            const float aDeg = minAngle + t * range;
            const float aRad = juce::degreesToRadians(aDeg);
            const float dx = cx + ringR * std::sin(aRad);
            const float dy = cy - ringR * std::cos(aRad);
            g.fillEllipse(dx - dotSize, dy - dotSize, dotSize * 2.0f, dotSize * 2.0f);
        }
    }

    void BMPanel::drawEndWords(juce::Graphics& g, float cx, float cy, float R,
                               const juce::String& leftWord, const juce::String& rightWord)
    {
        const float ringR = 1.62f * R;
        g.setFont(juce::Font(juce::FontOptions().withHeight(12.0f)));
        g.setColour(textMain.withAlpha(0.75f));
        {
            const float aRad = juce::degreesToRadians(-150.0f);
            const float lx = cx + ringR * std::sin(aRad);
            const float ly = cy - ringR * std::cos(aRad);
            g.drawText(leftWord, juce::Rectangle<float>(lx - 18.0f, ly - 10.0f, 36.0f, 20.0f),
                       juce::Justification::centred);
        }
        {
            const float aRad = juce::degreesToRadians(150.0f);
            const float lx = cx + ringR * std::sin(aRad);
            const float ly = cy - ringR * std::cos(aRad);
            g.drawText(rightWord, juce::Rectangle<float>(lx - 18.0f, ly - 10.0f, 36.0f, 20.0f),
                       juce::Justification::centred);
        }
    }

    void BMPanel::drawDiscreteLabels(juce::Graphics& g, float cx, float cy, float R,
                                     const juce::StringArray& labels,
                                     const juce::StringArray& angles,
                                     bool drawDots,
                                     const juce::StringArray& yOffsets)
    {
        const float rx = 1.55f * R;
        const float ry = 1.05f * R;
        g.setFont(juce::Font(juce::FontOptions().withHeight(12.0f)));
        g.setColour(textMain.withAlpha(0.85f));
        for (int i = 0; i < labels.size() && i < angles.size(); ++i)
        {
            const float aDeg = angles[i].getFloatValue();
            const float aRad = juce::degreesToRadians(aDeg);
            const float lx = cx + rx * std::sin(aRad);
            float ly = cy - ry * std::cos(aRad);
            if (i < yOffsets.size())
                ly += yOffsets[i].getFloatValue();
            g.drawText(labels[i], juce::Rectangle<float>(lx - 22.0f, ly - 8.0f, 44.0f, 16.0f),
                       juce::Justification::centred);
        }
        if (drawDots)
        {
            const float dotR = 1.26f * R;
            for (int i = 0; i < labels.size() && i < angles.size(); ++i)
            {
                const float aDeg = angles[i].getFloatValue();
                const float aRad = juce::degreesToRadians(aDeg);
                const float dx = cx + dotR * std::sin(aRad);
                const float dy = cy - dotR * std::cos(aRad);
                g.setColour(dotColour.withAlpha(0.85f));
                g.fillEllipse(dx - 2.5f, dy - 2.5f, 5.0f, 5.0f);
            }
        }
    }

    void BMPanel::drawScrew(juce::Graphics& g, float cx, float cy, float r, float rotationDeg)
    {
        juce::ColourGradient screwGrad(screwTop, cx - r, cy - r, screwBottom, cx + r, cy + r, false);
        g.setGradientFill(screwGrad);
        g.fillEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);
        g.setColour(screwBottom.darker(0.2f));
        g.drawEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f, 1.5f);
        {
            juce::Graphics::ScopedSaveState ss(g);
            g.addTransform(juce::AffineTransform::rotation(
                rotationDeg * juce::MathConstants<float>::pi / 180.0f, cx, cy));
            const float slotW = 2.5f, slotLen = r * 1.5f;
            g.setColour(screwSlotColour.withAlpha(0.7f));
            g.drawLine(cx - slotLen, cy - slotW * 0.5f, cx + slotLen, cy - slotW * 0.5f, slotW);
            g.drawLine(cx - slotW * 0.5f, cy - slotLen, cx - slotW * 0.5f, cy + slotLen, slotW);
        }
    }

    void BMPanel::drawWordmark(juce::Graphics& g, float cx, float cy, float capHeight)
    {
        juce::Font wmFont(juce::FontOptions().withHeight(capHeight * 0.85f).withStyle("bold"));
        juce::GlyphArrangement ga;
        ga.addLineOfText(wmFont, "bma", 0.0f, 0.0f);
        for (int i = 0, n = ga.getNumGlyphs(); i < n; ++i)
            ga.getGlyph(i).moveBy(static_cast<float>(i) * (-0.02f * capHeight), 0.0f);
        juce::Rectangle<float> wmBounds = ga.getBoundingBox(0, -1, true);
        ga.moveRangeOfGlyphs(0, -1, cx - wmBounds.getCentreX(), cy - wmBounds.getCentreY());
        juce::Path wmPath;
        ga.createPath(wmPath);
        g.setColour(textMain.withAlpha(0.75f));
        g.fillPath(wmPath);

        const float barBase = wmBounds.getBottom() - (cy - wmBounds.getCentreY());
        const float barW = 0.13f * capHeight;
        const float barGap = 0.09f * capHeight;
        const float heights[] = {0.45f, 0.62f, 0.78f, 0.92f, 1.00f};
        const float barStartX = wmBounds.getRight() + (cx - wmBounds.getCentreX()) + 8.0f;
        for (int i = 0; i < 5; ++i)
        {
            const float bh = heights[i] * capHeight;
            juce::Path bar;
            bar.addRoundedRectangle(barStartX + static_cast<float>(i) * (barW + barGap),
                barBase - bh, barW, bh, barW * 0.5f);
            g.fillPath(bar);
        }
    }

    void BMPanel::drawPanelText(juce::Graphics& g, float cx, float y, float size,
                                const juce::String& text, float trackingEm)
    {
        juce::Font font = resolveCondensedFont(size);
        bool needsScaling = !font.getTypefaceName().containsIgnoreCase("condensed")
                         && !font.getTypefaceName().containsIgnoreCase("narrow");
        {
            juce::Graphics::ScopedSaveState ss(g);
            if (needsScaling)
                g.addTransform(juce::AffineTransform::scale(0.88f, 1.0f).translated(cx * 0.136f, 0.0f));
            drawTrackedText(g, text.toUpperCase(), font, textMain.withAlpha(0.92f),
                           cx, y, trackingEm, textShadow.withAlpha(0.55f));
        }
    }
}
