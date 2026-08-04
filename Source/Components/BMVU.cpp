#include "BMVU.h"
#include "../GUI/BM176Colours.h"
#include "../GUI/BM176Geometry.h"

namespace bm176
{
    inline constexpr float DEG2RAD = juce::MathConstants<float>::pi / 180.0f;

    BMVU::BMVU()  { setRepaintsOnMouseActivity(false); }
    BMVU::~BMVU() { stopTimer(); }

    void BMVU::setTargetDB(float dB)
    {
        targetDB = juce::jlimit(-20.0f, 3.0f, dB);
        if (! juce::approximatelyEqual(targetDB, displayDB))
            startTimerHz(30);
    }
    void BMVU::setMode(bool gr)      { grMode = gr; }

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

    void BMVU::timerCallback()
    {
        constexpr float dt = 1.0f / 30.0f;
        constexpr float spring = 90.0f;
        constexpr float damping = 8.0f;
        constexpr float overshoot = 1.15f;

        const float err = targetDB - displayDB;
        velocity += spring * err * dt;
        velocity *= std::exp(-damping * dt);

        if (std::abs(err) < 0.01f && std::abs(velocity) < 0.05f)
        {
            displayDB = targetDB;
            velocity = 0.0f;
            stopTimer();
            repaint();
        }
        else
        {
            const float newVal = displayDB + velocity * dt;
            if (err > 0.0f && newVal > targetDB * overshoot)
                displayDB = targetDB * overshoot;
            else
                displayDB = newVal;
            repaint();
        }
    }

    void BMVU::resized()
    {
        faceCacheValid = false;
    }

    void BMVU::paint(juce::Graphics& g)
    {
        g.fillAll(juce::Colours::transparentBlack);

        glassX = VU_GLASS_X - VU_BEZEL_X;
        glassY = VU_GLASS_Y - VU_BEZEL_Y;
        glassW = VU_GLASS_W;
        glassH = VU_GLASS_H;

        if (!faceCacheValid || !cachedFace.isValid()
            || cachedFace.getWidth() != getWidth() || cachedFace.getHeight() != getHeight())
        {
            cachedFace = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
            juce::Graphics cg(cachedFace);
            drawStaticFace(cg);
            faceCacheValid = true;
        }
        g.drawImageAt(cachedFace, 0, 0);

        // Dynamic layers: needle + glare overlay
        {
            juce::Graphics::ScopedSaveState ss(g);
            g.reduceClipRegion(juce::roundToInt(glassX), juce::roundToInt(glassY),
                              juce::roundToInt(glassW), juce::roundToInt(glassH));
            g.addTransform(juce::AffineTransform::translation(glassX, glassY));
            drawNeedle(g);
        }
        drawGlare(g);
    }

    void BMVU::drawStaticFace(juce::Graphics& g)
    {
        drawBezel(g);
        drawGlassRecess(g);
        {
            juce::Graphics::ScopedSaveState ss(g);
            g.reduceClipRegion(juce::roundToInt(glassX), juce::roundToInt(glassY),
                              juce::roundToInt(glassW), juce::roundToInt(glassH));
            g.addTransform(juce::AffineTransform::translation(glassX, glassY));
            drawFaceBg(g);
            drawRedArc(g);
            drawDbScale(g);
            drawPctScale(g);
            drawFaceDecor(g);
        }
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

        g.setColour(juce::Colours::black.withAlpha(0.15f));
        g.strokePath(bezel, juce::PathStrokeType(1.5f));
    }

    void BMVU::drawGlassRecess(juce::Graphics& g)
    {
        juce::Path glassPath;
        glassPath.addRoundedRectangle(glassX, glassY, glassW, glassH, 3.0f);
        juce::DropShadow innerShadow(juce::Colours::black.withAlpha(0.75f), 12, juce::Point<int>(0, 1));
        innerShadow.drawForPath(g, glassPath);
    }

    void BMVU::drawFaceBg(juce::Graphics& g)
    {
        juce::Path face;
        face.addRoundedRectangle(0.0f, 0.0f, glassW, glassH, 3.0f);

        juce::ColourGradient faceGrad(
            juce::Colour(0xffffe8b0), glassW * 0.5f, glassH * 0.62f,
            juce::Colour(0xffdba040), 0.0f, glassH, true);
        g.setGradientFill(faceGrad);
        g.fillPath(face);

        juce::ColourGradient topShadow(
            juce::Colours::black.withAlpha(0.0f), glassW * 0.5f, glassH * 0.25f,
            juce::Colours::black.withAlpha(0.08f), glassW * 0.5f, 0.0f, false);
        g.setGradientFill(topShadow);
        g.fillPath(face);

        juce::ColourGradient warmTint(
            juce::Colour(0xff998050).withAlpha(0.03f), 0.0f, glassH * 0.6f,
            juce::Colour(0xff998050).withAlpha(0.0f), 0.0f, 0.0f, false);
        g.setGradientFill(warmTint);
        g.fillPath(face);
    }

    void BMVU::drawRedArc(juce::Graphics& g)
    {
        constexpr float Rred = 302.0f;
        const float cx = glassW * 0.5f;
        const float cy = 1.7f * glassH;

        const float startSpecDeg = pctToAngle(100.0f);
        const float endSpecDeg   = 33.0f;
        const float juceStart = juce::degreesToRadians(90.0f - startSpecDeg);
        const float juceEnd   = juce::degreesToRadians(90.0f - endSpecDeg);

        juce::Path arc;
        arc.addCentredArc(cx, cy, Rred, Rred, 0.0f, juceEnd, juceStart, true);
        g.setColour(vuRedArc.darker(0.1f));
        g.strokePath(arc, juce::PathStrokeType(7.0f, juce::PathStrokeType::curved, juce::PathStrokeType::butt));
    }

    void BMVU::drawDbScale(juce::Graphics& g)
    {
        constexpr float Rtick   = 268.0f;
        constexpr float RdBnum  = 284.0f;
        constexpr float pctVals[]  = {10.0f, 31.6f, 44.7f, 56.2f, 70.8f, 79.4f, 89.1f,
                                       100.0f, 112.2f, 125.9f, 141.3f};
        constexpr const char* labels[] = {"-20", "-10", "-7", "-5", "-3", "-2", "-1",
                                           "0", "+1", "+2", "+3"};
        constexpr int count = 11;
        constexpr int minorTickLen = 5;
        constexpr int majorTickLen = 9;

        juce::Colour tickColour = juce::Colour(0xff2a1e10);
        g.setFont(juce::Font(juce::FontOptions().withHeight(15.0f)));
        for (int i = 0; i < count; ++i)
        {
            const float angleDeg = pctToAngle(pctVals[i]);
            const int tickLen = (i == 2 || i == 4 || i == 5 || i == 6 || i == 8) ? minorTickLen : majorTickLen;
            const auto p1 = pointOnArc(Rtick - tickLen, angleDeg);
            const auto p2 = pointOnArc(Rtick, angleDeg);
            g.setColour(tickColour);
            g.drawLine(juce::Line<float>(p1, p2), tickLen == majorTickLen ? 1.4f : 0.7f);
            const auto lpos = pointOnArc(RdBnum, angleDeg);
            g.setColour(tickColour.darker(0.1f));
            g.drawText(labels[i],
                juce::Rectangle<float>(lpos.x - 16.0f, lpos.y - 10.0f, 32.0f, 20.0f),
                juce::Justification::centred);
        }

        for (float p = 10.0f; p <= 100.0f; p += 2.5f)
        {
            const float angleDeg = pctToAngle(p);
            const auto p1 = pointOnArc(Rtick - 5.0f, angleDeg);
            const auto p2 = pointOnArc(Rtick, angleDeg);
            g.setColour(tickColour.withAlpha(0.5f));
            g.drawLine(juce::Line<float>(p1, p2), 0.4f);
        }
        for (float p = 105.0f; p <= 140.0f; p += 5.0f)
        {
            const float angleDeg = pctToAngle(p);
            const auto p1 = pointOnArc(Rtick - 5.0f, angleDeg);
            const auto p2 = pointOnArc(Rtick, angleDeg);
            g.setColour(tickColour.withAlpha(0.5f));
            g.drawLine(juce::Line<float>(p1, p2), 0.4f);
        }
    }

    void BMVU::drawPctScale(juce::Graphics& g)
    {
        constexpr float RpctTck = 254.0f;
        constexpr float Rpctnum = 244.0f;
        juce::Colour tickColour = juce::Colour(0xff2a1e10);

        g.setFont(juce::Font(juce::FontOptions().withHeight(12.0f)));
        for (int p = 0; p <= 100; p += 20)
        {
            const float angleDeg = pctToAngle(static_cast<float>(p));
            const auto p1 = pointOnArc(RpctTck - 3.0f, angleDeg);
            const auto p2 = pointOnArc(RpctTck, angleDeg);
            g.setColour(tickColour);
            g.drawLine(juce::Line<float>(p1, p2), 0.8f);
            const auto lpos = pointOnArc(Rpctnum, angleDeg);
            g.setColour(tickColour.darker(0.1f));
            g.drawText(juce::String(p),
                juce::Rectangle<float>(lpos.x - 14.0f, lpos.y - 9.0f, 28.0f, 18.0f),
                juce::Justification::centred);
        }
    }

    void BMVU::drawFaceDecor(juce::Graphics& g)
    {
        const float cx = glassW * 0.5f;
        const float markY = glassH - 30.0f;
        const float capH = 26.0f;

        juce::Font wmFont(juce::FontOptions().withHeight(capH));
        juce::GlyphArrangement ga;
        ga.addLineOfText(wmFont, "bma", 0.0f, 0.0f);
        juce::Rectangle<float> wmBounds = ga.getBoundingBox(0, -1, true);

        const float barW = 0.13f * capH;
        const float barGap = 0.09f * capH;
        const float barStartOffset = 6.0f;
        const float totalBarW = 5.0f * barW + 4.0f * barGap;
        const float unitWidth = wmBounds.getWidth() + barStartOffset + totalBarW;
        const float unitLeft = cx - unitWidth * 0.5f;
        const float barBase = markY + wmBounds.getHeight() * 0.4f;

        ga.moveRangeOfGlyphs(0, -1, unitLeft - wmBounds.getX(), markY - wmBounds.getCentreY());
        juce::Path wmPath;
        ga.createPath(wmPath);
        g.setColour(vuWordmark.withAlpha(0.75f));
        g.fillPath(wmPath);

        const float barStartX = unitLeft + wmBounds.getWidth() + barStartOffset;
        const float heights[] = {0.45f, 0.62f, 0.78f, 0.92f, 1.00f};
        for (int i = 0; i < 5; ++i)
        {
            const float bh = heights[i] * capH;
            juce::Path bar;
            bar.addRoundedRectangle(barStartX + static_cast<float>(i) * (barW + barGap),
                barBase - bh, barW, bh, barW * 0.5f);
            g.setColour(vuWordmark.withAlpha(0.60f));
            g.fillPath(bar);
        }

        constexpr float lampW = 34.0f;
        constexpr float lampH = 12.0f;
        constexpr float lampCy = 181.0f;
        g.setColour(vuLampBg.withAlpha(0.65f));
        for (int i = 0; i < 3; ++i)
        {
            const float lx = cx + (static_cast<float>(i) - 1.0f) * 41.0f;
            juce::Path lp;
            lp.addRoundedRectangle(lx - lampW * 0.5f, lampCy - lampH * 0.5f, lampW, lampH, 3.0f);
            g.fillPath(lp);
        }
    }

    void BMVU::drawNeedle(juce::Graphics& g)
    {
        constexpr float Rtick = 268.0f;
        float pct = 100.0f * std::pow(10.0f, displayDB / 20.0f);
        if (grMode) pct = 100.0f - pct;
        pct = juce::jlimit(0.0f, 150.0f, pct);
        const float angleDeg = pctToAngle(pct);
        const float angleRad = angleDeg * DEG2RAD;
        const float cx = glassW * 0.5f;
        const float cy = 1.7f * glassH;

        const float hubR = 0.28f * Rtick;
        const float tipR = 1.03f * Rtick;
        constexpr float hubW = 3.5f;
        constexpr float tipW = 1.6f;

        const float cosA = std::cos(angleRad);
        const float sinA = std::sin(angleRad);
        const float midX = cx + hubR * sinA;
        const float midY = cy - hubR * cosA;
        const float tipX = cx + tipR * sinA;
        const float tipY = cy - tipR * cosA;

        const float perpX = cosA * 0.5f;
        const float perpY = sinA * 0.5f;

        juce::Path needle;
        needle.startNewSubPath(midX - perpX * hubW, midY - perpY * hubW);
        needle.lineTo(midX + perpX * hubW, midY + perpY * hubW);
        needle.lineTo(tipX + perpX * tipW, tipY + perpY * tipW);
        needle.lineTo(tipX - perpX * tipW, tipY - perpY * tipW);
        needle.closeSubPath();

        needle.applyTransform(juce::AffineTransform::translation(2.0f, 3.0f));
        g.setColour(juce::Colours::black.withAlpha(0.25f));
        g.fillPath(needle);

        needle.applyTransform(juce::AffineTransform::translation(-2.0f, -3.0f));
        g.setColour(juce::Colour(0xff141210));
        g.fillPath(needle);
    }

    void BMVU::drawGlare(juce::Graphics& g)
    {
        juce::Path glass;
        glass.addRoundedRectangle(glassX, glassY, glassW, glassH, 3.0f);

        juce::ColourGradient reflection(
            juce::Colours::white.withAlpha(0.06f), glassX, glassY,
            juce::Colours::white.withAlpha(0.0f), glassX, glassY + glassH * 0.45f, false);
        g.setGradientFill(reflection);
        g.fillPath(glass);

        juce::ColourGradient fresnel(
            juce::Colours::white.withAlpha(0.02f), glassX + glassW * 0.5f, glassY + glassH * 0.5f,
            juce::Colours::white.withAlpha(0.0f), glassX, glassY + glassH * 0.5f, false);
        g.setGradientFill(fresnel);
        g.fillPath(glass);
    }
}
