#include "BM176HardwareSwitch.h"
#include "../GUI/BM176Colours.h"

namespace bm176
{
    BM176HardwareSwitch::BM176HardwareSwitch()
    {
        setRepaintsOnMouseActivity(false);
        startTimerHz(60);
    }

    BM176HardwareSwitch::~BM176HardwareSwitch() { stopTimer(); }

    void BM176HardwareSwitch::setState(bool on)
    {
        if (state != on)
        {
            state = on;
            targetY = state ? slotBottomY() : slotTopY();
            if (!dragging)
                velocity += state ? 80.0f : -80.0f;
        }
    }

    bool BM176HardwareSwitch::getState() const { return state; }

    void BM176HardwareSwitch::setLabels(const juce::String& t,
                                        const juce::String& top,
                                        const juce::String& bottom)
    {
        title = t;
        topLabel = top;
        bottomLabel = bottom;
        repaint();
    }

    float BM176HardwareSwitch::slotTopY() const
    {
        return getHeight() * 0.45f;
    }

    float BM176HardwareSwitch::slotBottomY() const
    {
        return slotTopY() + slotTravel();
    }

    float BM176HardwareSwitch::slotTravel() const
    {
        return 12.0f;
    }

    float BM176HardwareSwitch::clampSlot(float y) const
    {
        return juce::jlimit(slotTopY(), slotBottomY(), y);
    }

    void BM176HardwareSwitch::mouseDown(const juce::MouseEvent& e)
    {
        dragging = true;
        dragOrigin = actuatorY;
        dragY = e.y;

        const float midpoint = (slotTopY() + slotBottomY()) * 0.5f;
        setState(actuatorY > midpoint);
    }

    void BM176HardwareSwitch::mouseDrag(const juce::MouseEvent& e)
    {
        if (!dragging) return;
        const float delta = e.y - dragY;
        actuatorY = clampSlot(dragOrigin + delta * 0.5f);
        repaint();
    }

    void BM176HardwareSwitch::mouseUp(const juce::MouseEvent& e)
    {
        dragging = false;

        const float midpoint = (slotTopY() + slotBottomY()) * 0.5f;
        setState(actuatorY > midpoint);
    }

    void BM176HardwareSwitch::timerCallback()
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

    void BM176HardwareSwitch::resized()
    {
        targetY = state ? slotBottomY() : slotTopY();
        actuatorY = targetY;
    }

    void BM176HardwareSwitch::paint(juce::Graphics& g)
    {
        const float w = static_cast<float>(getWidth());
        const float h = static_cast<float>(getHeight());
        const float cx = w * 0.5f;

        const float bezelW = 28.0f;
        const float bezelH = 42.0f;
        const float bezelX = cx - bezelW * 0.5f;
        const float bezelY = h * 0.5f - bezelH * 0.5f;
        const float bezelR = 4.0f;

        juce::Path bezel;
        bezel.addRoundedRectangle(bezelX, bezelY, bezelW, bezelH, bezelR);
        juce::DropShadow bezelShadow(juce::Colours::black.withAlpha(0.35f), 5, juce::Point<int>(1, 3));
        bezelShadow.drawForPath(g, bezel);

        juce::ColourGradient bezelGrad(
            panelBase.darker(0.15f), bezelX, bezelY,
            panelBase.darker(0.25f), bezelX, bezelY + bezelH, false);
        g.setGradientFill(bezelGrad);
        g.fillPath(bezel);

        g.setColour(screwBottom.darker(0.3f).withAlpha(0.5f));
        g.strokePath(bezel, juce::PathStrokeType(1.0f));

        const float cavityW = 24.0f;
        const float cavityH = 26.0f;
        const float cavityX = cx - cavityW * 0.5f;
        const float cavityY = bezelY + 8.0f;
        const float cavityR = 3.0f;

        juce::Path cavity;
        cavity.addRoundedRectangle(cavityX, cavityY, cavityW, cavityH, cavityR);
        g.setColour(juce::Colours::black);
        g.fillPath(cavity);

        juce::DropShadow cavityShadow(juce::Colours::black.withAlpha(0.4f), 3, juce::Point<int>(0, 1));
        cavityShadow.drawForPath(g, cavity);

        constexpr float actuatorH = 14.0f;
        const float actuatorW = 24.0f;
        const float actX = cx - actuatorW * 0.5f;
        const float actY = actuatorY - actuatorH * 0.5f;
        const float actR = 2.0f;

        juce::Path actuator;
        actuator.addRoundedRectangle(actX, actY, actuatorW, actuatorH, actR);

        juce::DropShadow actShadow(juce::Colours::black.withAlpha(0.35f), 3, juce::Point<int>(0, 2));
        actShadow.drawForPath(g, actuator);

        juce::ColourGradient metalGrad(
            chromeTop.brighter(0.1f), 0.0f, actY,
            screwBottom, 0.0f, actY + actuatorH, false);
        g.setGradientFill(metalGrad);
        g.fillPath(actuator);

        g.setColour(chromeTop.brighter(0.3f).withAlpha(0.8f));
        g.drawHorizontalLine(juce::roundToInt(actY) + 1,
            juce::roundToInt(actX) + 3,
            juce::roundToInt(actX + actuatorW) - 3);

        g.setColour(screwBottom.darker(0.3f).withAlpha(0.4f));
        g.strokePath(actuator, juce::PathStrokeType(0.8f));

        if (title.isNotEmpty())
        {
            g.setFont(juce::Font(juce::FontOptions().withHeight(9.0f)));
            g.setColour(textMain.withAlpha(0.5f));
            g.drawText(title, juce::Rectangle<float>(0.0f, 2.0f, w, 12.0f), juce::Justification::centred);
        }
    }
}
