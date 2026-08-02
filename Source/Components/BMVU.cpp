#include "BMVU.h"
#include "../GUI/BM176Colours.h"
#include "../GUI/BM176Geometry.h"

namespace bm176
{
    inline constexpr float DEG2RAD = juce::MathConstants<float>::pi / 180.0f;

    BMVU::BMVU()
    {
        setRepaintsOnMouseActivity(false);
    }

    void BMVU::setValue(float dB)    { needleDB = juce::jlimit(-20.0f, 3.0f, dB); repaint(); }
    void BMVU::setMode(bool gr)      { grMode = gr; repaint(); }

    float BMVU::pctToAngle(float pct) const
    {
        return -44.0f + 0.53f * pct;
    }

    juce::Point<float> BMVU::pointOnArc(float radius, float angleDeg) const
    {
        const float rad = angleDeg * DEG2RAD;
        const float cx = glassW * 0.5f;
        const float cy = 1.7f * glassH;
        return { cx + radius * std::sin(rad), cy - radius * std::cos(rad) };
    }

    void BMVU::resized() {}

    void BMVU::paint(juce::Graphics& g)
    {
        g.fillAll(juce::Colours::transparentBlack);

        const auto b = getLocalBounds().toFloat();
        glassX = VU_GLASS_X - VU_BEZEL_X;
        glassY = VU_GLASS_Y - VU_BEZEL_Y;
        glassW = VU_GLASS_W;
        glassH = VU_GLASS_H;

        drawBezel(g);
        drawGlassRecess(g);

        juce::Graphics::ScopedSaveState ss(g);
        g.reduceClipRegion(glassX, glassY, static_cast<int>(glassW), static_cast<int>(glassH));
        g.addTransform(juce::AffineTransform::translation(glassX, glassY));

        drawFaceBg(g);
        drawRedArc(g);
        drawDbScale(g);
        drawPctScale(g);
        drawNeedle(g);
        drawFaceDecor(g);

        g.addTransform(juce::AffineTransform::translation(-glassX, -glassY));
        drawGlare(g);
    }

    void BMVU::drawBezel(juce::Graphics& g)
    {
        juce::Path bezel;
        bezel.addRoundedRectangle(0.0f, 0.0f, static_cast<float>(getWidth()),
            static_cast<float>(getHeight()), 8.0f);
        juce::ColourGradient bezelGrad(vuBezelTop, 0.0f, 0.0f,
                                       vuBezelBottom, 0.0f, static_cast<float>(getHeight()), false);
        g.setGradientFill(bezelGrad);
        g.fillPath(bezel);
    }

    void BMVU::drawGlassRecess(juce::Graphics& g)
    {
        juce::Path glassPath;
        glassPath.addRoundedRectangle(glassX, glassY, glassW, glassH, 3.0f);
        juce::DropShadow innerShadow(juce::Colours::black.withAlpha(0.70f), 10, juce::Point<int>(0, 0));
        innerShadow.drawForPath(g, glassPath);
    }

    void BMVU::drawFaceBg(juce::Graphics& g)
    {
        juce::Path face;
        face.addRoundedRectangle(0.0f, 0.0f, glassW, glassH, 3.0f);
        juce::ColourGradient faceGrad(vuFaceCenter, glassW * 0.5f, glassH * 0.62f,
                                      vuFaceCorner, 0.0f, glassH, true);
        g.setGradientFill(faceGrad);
        g.fillPath(face);

        juce::ColourGradient topShadow(
            juce::Colours::black.withAlpha(0.0f), glassW * 0.5f, glassH * 0.3f,
            juce::Colours::black.withAlpha(0.06f), glassW * 0.5f, 0.0f, false);
        g.setGradientFill(topShadow);
        g.fillPath(face);
    }

    void BMVU::drawRedArc(juce::Graphics& g)
    {
        constexpr float Rred = 302.0f;
        const float startAngle = pctToAngle(100.0f);
        constexpr float endAngle = 33.0f;

        juce::Path arc;
        const float cx = glassW * 0.5f;
        const float cy = 1.7f * glassH;
        arc.addCentredArc(cx, cy, Rred, Rred, 0.0f,
            juce::degreesToRadians(startAngle), juce::degreesToRadians(endAngle), true);
        g.setColour(vuRedArc);
        g.strokePath(arc, juce::PathStrokeType(7.0f, juce::PathStrokeType::curved, juce::PathStrokeType::butt));
    }

    void BMVU::drawDbScale(juce::Graphics& g)
    {
        constexpr float Rtick   = 268.0f;
        constexpr float RdBnum  = 284.0f;
        constexpr float pctVals[] = {10.0f, 31.6f, 44.7f, 56.2f, 70.8f, 79.4f, 89.1f,
                                      100.0f, 112.2f, 125.9f, 141.3f};
        constexpr const char* labels[] = {"-20", "-10", "-7", "-5", "-3", "-2", "-1",
                                           "0", "+1", "+2", "+3"};
        constexpr int count = 11;
        constexpr bool major[] = {true, true, false, true, false, false, false,
                                   true, false, true, true};

        g.setFont(juce::Font(juce::FontOptions().withHeight(15.0f)));
        for (int i = 0; i < count; ++i)
        {
            const float angleDeg = pctToAngle(pctVals[i]);
            const auto p1 = pointOnArc(Rtick - (major[i] ? 9.0f : 5.0f), angleDeg);
            const auto p2 = pointOnArc(Rtick, angleDeg);

            g.setColour(vuNumeral);
            g.drawLine(juce::Line<float>(p1, p2), major[i] ? 1.2f : 0.6f);

            if (major[i])
            {
                const auto lpos = pointOnArc(RdBnum, angleDeg);
                g.drawText(labels[i],
                    juce::Rectangle<float>(lpos.x - 16.0f, lpos.y - 10.0f, 32.0f, 20.0f),
                    juce::Justification::centred);
            }
        }

        for (float p = 10.0f; p <= 100.0f; p += 2.5f)
        {
            const float angleDeg = pctToAngle(p);
            const auto p1 = pointOnArc(Rtick - 5.0f, angleDeg);
            const auto p2 = pointOnArc(Rtick, angleDeg);
            g.setColour(vuNumeral.withAlpha(0.5f));
            g.drawLine(juce::Line<float>(p1, p2), 0.4f);
        }
        for (float p = 105.0f; p <= 140.0f; p += 5.0f)
        {
            const float angleDeg = pctToAngle(p);
            const auto p1 = pointOnArc(Rtick - 5.0f, angleDeg);
            const auto p2 = pointOnArc(Rtick, angleDeg);
            g.setColour(vuNumeral.withAlpha(0.5f));
            g.drawLine(juce::Line<float>(p1, p2), 0.4f);
        }
    }

    void BMVU::drawPctScale(juce::Graphics& g)
    {
        constexpr float RpctTck = 254.0f;
        constexpr float Rpctnum = 244.0f;

        g.setFont(juce::Font(juce::FontOptions().withHeight(12.0f)));
        for (int p = 0; p <= 100; p += 20)
        {
            const float angleDeg = pctToAngle(static_cast<float>(p));
            const auto p1 = pointOnArc(RpctTck - 3.0f, angleDeg);
            const auto p2 = pointOnArc(RpctTck, angleDeg);
            g.setColour(vuNumeral);
            g.drawLine(juce::Line<float>(p1, p2), 0.8f);

            const auto lpos = pointOnArc(Rpctnum, angleDeg);
            g.drawText(juce::String(p),
                juce::Rectangle<float>(lpos.x - 14.0f, lpos.y - 9.0f, 28.0f, 18.0f),
                juce::Justification::centred);
        }
    }

    void BMVU::drawNeedle(juce::Graphics& g)
    {
        constexpr float Rtick = 268.0f;
        float pct = 100.0f * std::pow(10.0f, needleDB / 20.0f);
        if (grMode) pct = 100.0f - pct;
        pct = juce::jlimit(0.0f, 150.0f, pct);
        const float angleDeg = pctToAngle(pct);
        const float angleRad = angleDeg * DEG2RAD;
        const float cx = glassW * 0.5f;
        const float cy = 1.7f * glassH;

        const float hubR = 0.28f * Rtick;
        const float tipR = 1.03f * Rtick;
        const float x1 = cx + hubR * std::sin(angleRad);
        const float y1 = cy - hubR * std::cos(angleRad);
        const float x2 = cx + tipR * std::sin(angleRad);
        const float y2 = cy - tipR * std::cos(angleRad);

        juce::Path needle;
        needle.addLineSegment(juce::Line<float>(x1, y1, x2, y2), 0.0f);
        juce::PathStrokeType nStroke(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);

        juce::Path shadowP;
        nStroke.createStrokedPath(shadowP, needle, juce::AffineTransform::translation(2.0f, 3.0f));
        g.setColour(juce::Colours::black.withAlpha(0.30f));
        g.fillPath(shadowP);

        g.setColour(juce::Colour(0xff161616));
        g.strokePath(needle, nStroke);

        juce::Path thinNeedle;
        thinNeedle.addLineSegment(juce::Line<float>(x1, y1, x2, y2), 0.0f);
        juce::PathStrokeType thinS(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
        g.setColour(juce::Colour(0xff161616));
        g.strokePath(thinNeedle, thinS);
    }

    void BMVU::drawFaceDecor(juce::Graphics& g)
    {
        const float cx = glassW * 0.5f;
        const float markY = 113.0f;
        const float markH = 26.0f;

        juce::Font wmFont(juce::FontOptions().withHeight(markH * 0.85f).withStyle("bold"));
        g.setFont(wmFont);
        g.setColour(vuWordmark.withAlpha(0.85f));
        g.drawText("bma",
            juce::Rectangle<float>(cx - 30.0f, markY - markH * 0.5f, 24.0f, markH),
            juce::Justification::centredLeft);

        g.setColour(vuWordmark.withAlpha(0.7f));
        juce::Font smallFont(juce::FontOptions().withHeight(markH * 0.42f));
        g.setFont(smallFont);
        g.drawText("GR", juce::Rectangle<float>(cx - 14.0f, markY + markH * 0.15f, 28.0f, 14.0f),
            juce::Justification::centred);

        constexpr float lampW = 34.0f;
        constexpr float lampH = 12.0f;
        constexpr float lampY = 181.0f;
        g.setColour(vuLampBg.withAlpha(0.70f));
        for (int i = 0; i < 3; ++i)
        {
            const float lx = cx + (static_cast<float>(i) - 1.0f) * 41.0f;
            juce::Path lp;
            lp.addRoundedRectangle(lx - lampW * 0.5f, lampY, lampW, lampH, 3.0f);
            g.fillPath(lp);
        }
    }

    void BMVU::drawGlare(juce::Graphics& g)
    {
        juce::Path glass;
        glass.addRoundedRectangle(glassX, glassY, glassW, glassH, 3.0f);
        juce::ColourGradient reflection(
            juce::Colours::white.withAlpha(0.04f), glassX, glassY,
            juce::Colours::white.withAlpha(0.0f), glassX, glassY + glassH * 0.4f, false);
        g.setGradientFill(reflection);
        g.fillPath(glass);
    }
}
