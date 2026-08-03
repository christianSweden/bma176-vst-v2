#include "BMVerticalSlideSwitch.h"
#include "../GUI/BM176Colours.h"
#include "../GUI/BM176Colours.h"

namespace bm176
{
    BMVerticalSlideSwitch::BMVerticalSlideSwitch()
    {
        setRepaintsOnMouseActivity(false);
        startTimerHz(60);
    }

    BMVerticalSlideSwitch::~BMVerticalSlideSwitch() { stopTimer(); }

    void BMVerticalSlideSwitch::setState(bool on)
    {
        if (state != on)
        {
            state = on;
            targetY = state ? slotBottomY() : slotTopY();
            if (!dragging)
                velocity += state ? 80.0f : -80.0f;
        }
    }

    bool BMVerticalSlideSwitch::getState() const { return state; }

    void BMVerticalSlideSwitch::setLabels(const juce::String& t,
                                          const juce::String& top,
                                          const juce::String& bottom)
    {
        title = t;
        topLabel = top;
        bottomLabel = bottom;
        repaint();
    }

    float BMVerticalSlideSwitch::slotTopY() const
    {
        return getHeight() * 0.38f;
    }

    float BMVerticalSlideSwitch::slotBottomY() const
    {
        return getHeight() * 0.38f + slotTravel();
    }

    float BMVerticalSlideSwitch::slotTravel() const
    {
        return getHeight() * 0.35f;
    }

    float BMVerticalSlideSwitch::clampSlot(float y) const
    {
        return juce::jlimit(slotTopY(), slotBottomY(), y);
    }

    void BMVerticalSlideSwitch::mouseDown(const juce::MouseEvent& e)
    {
        dragging = true;
        dragOrigin = actuatorY;
        dragY = e.y;

        const float midpoint = (slotTopY() + slotBottomY()) * 0.5f;
        setState(actuatorY > midpoint);
    }

    void BMVerticalSlideSwitch::mouseDrag(const juce::MouseEvent& e)
    {
        if (!dragging) return;
        const float delta = e.y - dragY;
        actuatorY = clampSlot(dragOrigin + delta * 0.5f);
        repaint();
    }

    void BMVerticalSlideSwitch::mouseUp(const juce::MouseEvent& e)
    {
        dragging = false;

        const float midpoint = (slotTopY() + slotBottomY()) * 0.5f;
        setState(actuatorY > midpoint);
    }

    void BMVerticalSlideSwitch::timerCallback()
    {
        if (dragging) return;

        constexpr float dt = 1.0f / 60.0f;
        constexpr float spring = 200.0f;
        constexpr float damping = 14.0f;

        const float force = spring * (targetY - actuatorY);
        velocity += force * dt;
        velocity *= std::exp(-damping * dt);

        const float prevY = actuatorY;
        actuatorY += velocity * dt;
        actuatorY = clampSlot(actuatorY);

        if (std::abs(targetY - actuatorY) < 0.15f && std::abs(velocity) < 0.5f)
        {
            actuatorY = targetY;
            velocity = 0.0f;
        }

        if (std::abs(actuatorY - prevY) > 0.05f)
            repaint();
    }

    void BMVerticalSlideSwitch::resized()
    {
        targetY = state ? slotBottomY() : slotTopY();
        actuatorY = targetY;
    }

    void BMVerticalSlideSwitch::paint(juce::Graphics& g)
    {
        const float w = static_cast<float>(getWidth());
        const float h = static_cast<float>(getHeight());

        const float enclosureX = w * 0.12f;
        const float enclosureW = w * 0.76f;
        const float enclosureY = h * 0.18f;
        const float enclosureH = h * 0.64f;
        const float enclosureR = 4.0f;

        juce::Path enclosure;
        enclosure.addRoundedRectangle(enclosureX, enclosureY, enclosureW, enclosureH, enclosureR);
        juce::DropShadow encShadow(juce::Colours::black.withAlpha(0.35f), 6, juce::Point<int>(1, 3));
        encShadow.drawForPath(g, enclosure);

        juce::ColourGradient encGrad(
            panelBase.darker(0.15f), enclosureX, enclosureY,
            panelBase.darker(0.25f), enclosureX, enclosureY + enclosureH, false);
        g.setGradientFill(encGrad);
        g.fillPath(enclosure);

        g.setColour(screwBottom.darker(0.3f).withAlpha(0.5f));
        g.strokePath(enclosure, juce::PathStrokeType(1.2f));

        const float slotX = enclosureX + 5.0f;
        const float slotW = enclosureW - 10.0f;
        const float slotY = slotTopY();
        const float slotH = slotTravel();
        const float slotR = 3.0f;

        juce::Path slot;
        slot.addRoundedRectangle(slotX, slotY, slotW, slotH, slotR);
        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.fillPath(slot);

        juce::DropShadow slotShadow(juce::Colours::black.withAlpha(0.5f), 4, juce::Point<int>(0, 2));
        slotShadow.drawForPath(g, slot);

        const float actuatorH = 18.0f;
        const float actY = actuatorY - actuatorH * 0.5f;
        const float actX = slotX + 1.5f;
        const float actW = slotW - 3.0f;

        juce::Path actuator;
        actuator.addRoundedRectangle(actX, actY, actW, actuatorH, 3.0f);

        juce::DropShadow actShadow(juce::Colours::black.withAlpha(0.4f), 3, juce::Point<int>(0, 2));
        actShadow.drawForPath(g, actuator);

        juce::ColourGradient actGrad(
            chromeTop.brighter(0.15f), actX, actY,
            screwBottom, actX, actY + actuatorH, false);
        g.setGradientFill(actGrad);
        g.fillPath(actuator);

        g.setColour(chromeTop.brighter(0.3f));
        g.drawRoundedRectangle(actX + 0.5f, actY + 0.5f, actW - 1.0f, actuatorH * 0.4f, 2.0f, 1.0f);

        g.setColour(screwBottom.darker(0.3f).withAlpha(0.5f));
        g.strokePath(actuator, juce::PathStrokeType(1.0f));

        if (title.isNotEmpty())
        {
            g.setFont(juce::Font(juce::FontOptions().withHeight(11.0f)));
            g.setColour(textMain.withAlpha(0.65f));
            g.drawText(title, juce::Rectangle<float>(0.0f, 2.0f, w, 14.0f), juce::Justification::centred);
        }
        if (topLabel.isNotEmpty())
        {
            g.setFont(juce::Font(juce::FontOptions().withHeight(9.0f)));
            g.setColour(textMain.withAlpha(0.5f));
            g.drawText(topLabel, juce::Rectangle<float>(slotX + slotW + 2.0f, slotY, w - slotX - slotW, 10.0f),
                       juce::Justification::centredLeft);
        }
        if (bottomLabel.isNotEmpty())
        {
            g.setFont(juce::Font(juce::FontOptions().withHeight(9.0f)));
            g.setColour(textMain.withAlpha(0.5f));
            g.drawText(bottomLabel, juce::Rectangle<float>(slotX + slotW + 2.0f, slotY + slotH - 10.0f, w - slotX - slotW, 10.0f),
                       juce::Justification::centredLeft);
        }
    }
}
