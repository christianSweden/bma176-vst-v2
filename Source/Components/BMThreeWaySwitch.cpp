#include "BMThreeWaySwitch.h"
#include "../GUI/BM176Colours.h"

namespace bm176
{
    BMThreeWaySwitch::BMThreeWaySwitch()
    {
        setRepaintsOnMouseActivity(false);
    }

    void BMThreeWaySwitch::setPosition(int pos)
    {
        pos = juce::jlimit(0, 2, pos);
        if (position != pos)
        {
            position = pos;
            repaint();
        }
    }

    int BMThreeWaySwitch::getPosition() const { return position; }

    void BMThreeWaySwitch::setLabels(const juce::String& t,
                                     const juce::String& l,
                                     const juce::String& c,
                                     const juce::String& r)
    {
        title = t;
        leftText = l;
        centerText = c;
        rightText = r;
        repaint();
    }

    void BMThreeWaySwitch::mouseDown(const juce::MouseEvent& e)
    {
        const float third = static_cast<float>(getWidth()) / 3.0f;
        if (e.x < third)
            setPosition(0);
        else if (e.x < third * 2.0f)
            setPosition(1);
        else
            setPosition(2);
    }

    void BMThreeWaySwitch::resized() {}

    void BMThreeWaySwitch::paint(juce::Graphics& g)
    {
        const float w = static_cast<float>(getWidth());
        const float h = static_cast<float>(getHeight());
        const float cx = w * 0.5f;
        const float bushingY = h * 0.55f;
        const float titleY = h * 0.08f;
        const float labelY = h * 0.78f;

        g.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), w * 0.28f, juce::Font::plain));
        g.setColour(textWhite);
        g.drawText(title, juce::Rectangle<float>(0.0f, titleY, w, h * 0.20f), juce::Justification::centred);

        drawBushing(g, cx, bushingY);
        drawLever(g, cx, bushingY);

        juce::String label;
        if (position == 0)      label = leftText;
        else if (position == 1) label = centerText;
        else                    label = rightText;

        g.setColour(textWhite.withAlpha(0.85f));
        g.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), w * 0.22f, juce::Font::plain));
        g.drawText(label, juce::Rectangle<float>(0.0f, labelY, w, h * 0.15f), juce::Justification::centred);
    }

    void BMThreeWaySwitch::drawBushing(juce::Graphics& g, float cx, float by)
    {
        const float size = juce::jmin(getWidth(), getHeight()) * 0.30f;

        juce::Path washer;
        washer.addEllipse(juce::Rectangle<float>(cx - size, by - size * 0.55f, size * 2.0f, size * 1.1f));
        juce::ColourGradient washerGrad(chassisTop.brighter(0.2f), cx - size, by,
                                        chassisMetal, cx + size, by, false);
        g.setGradientFill(washerGrad);
        g.fillPath(washer);
        g.setColour(chassisMetal.darker(0.3f));
        g.drawEllipse(cx - size, by - size * 0.55f, size * 2.0f, size * 1.1f, 0.8f);

        const float nutR = size * 0.65f;
        juce::Path nut;
        nut.addEllipse(juce::Rectangle<float>(cx - nutR, by - nutR * 0.7f, nutR * 2.0f, nutR * 1.4f));
        juce::ColourGradient nutGrad(screwMetal.brighter(0.25f), cx - nutR, by - nutR,
                                     screwMetal.darker(0.1f), cx + nutR, by + nutR, false);
        g.setGradientFill(nutGrad);
        g.fillPath(nut);
    }

    void BMThreeWaySwitch::drawLever(juce::Graphics& g, float cx, float by)
    {
        const float size = juce::jmin(getWidth(), getHeight()) * 0.30f;
        const float leverLength = size * 3.0f;
        const float angles[3] = {
            juce::degreesToRadians(-30.0f),
            0.0f,
            juce::degreesToRadians(30.0f)
        };
        const float leverAngle = angles[position];
        const float tipRadius = size * 0.38f;

        const float ex = cx + leverLength * std::sin(leverAngle);
        const float ey = by - leverLength * std::cos(leverAngle);

        juce::Path stem;
        stem.addLineSegment(juce::Line<float>(cx, by, ex, ey), size * 0.22f);
        juce::ColourGradient stemGrad(screwMetal.brighter(0.3f), ex, ey,
                                      screwMetal, cx, by, false);
        g.setGradientFill(stemGrad);
        g.strokePath(stem, juce::PathStrokeType(size * 0.28f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::ColourGradient tipGrad(screwMetal.brighter(0.4f), ex - tipRadius, ey - tipRadius,
                                     screwMetal, ex + tipRadius, ey + tipRadius, false);
        juce::Path tip;
        tip.addEllipse(juce::Rectangle<float>(ex - tipRadius, ey - tipRadius, tipRadius * 2.0f, tipRadius * 2.0f));
        g.setGradientFill(tipGrad);
        g.fillPath(tip);
        g.setColour(screwMetal.darker(0.2f));
        g.drawEllipse(ex - tipRadius, ey - tipRadius, tipRadius * 2.0f, tipRadius * 2.0f, 0.8f);
    }
}
