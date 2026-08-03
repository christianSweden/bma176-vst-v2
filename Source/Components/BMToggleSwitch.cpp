#include "BMToggleSwitch.h"
#include "../GUI/BM176Colours.h"

namespace bm176
{
    BMToggleSwitch::BMToggleSwitch()
    {
        setRepaintsOnMouseActivity(false);
        startTimerHz(60);
    }

    BMToggleSwitch::~BMToggleSwitch() { stopTimer(); }

    void BMToggleSwitch::setState(bool up)
    {
        if (state != up)
        {
            state = up;
            animTarget = state ? 1.0f : 0.0f;
            animVelocity += state ? 0.18f : -0.18f;
        }
    }

    bool BMToggleSwitch::getState() const { return state; }

    void BMToggleSwitch::mouseDown(const juce::MouseEvent&)
    {
        setState(!state);
    }

    void BMToggleSwitch::timerCallback()
    {
        constexpr float dt = 1.0f / 60.0f;
        constexpr float spring = 180.0f;
        constexpr float damping = 12.0f;

        const float force = spring * (animTarget - animT);
        animVelocity += force * dt;
        animVelocity *= std::exp(-damping * dt);

        if (std::abs(animTarget - animT) < 0.0005f && std::abs(animVelocity) < 0.001f)
        {
            animT = animTarget;
            animVelocity = 0.0f;
            return;
        }

        animT += animVelocity * dt;
        animT = juce::jlimit(0.0f, 1.0f, animT);
        repaint();
    }

    void BMToggleSwitch::resized() {}

    void BMToggleSwitch::paint(juce::Graphics& g)
    {
        const float cx = getWidth() * 0.5f;
        const float cy = getHeight() * 0.5f;
        const float nutR = 17.0f;
        const float nutH = 14.0f;

        juce::Path shadowPath;
        shadowPath.addEllipse(juce::Rectangle<float>(cx - nutR * 0.9f, cy - nutR * 0.6f,
            nutR * 1.8f, nutR * 1.2f));
        juce::DropShadow baseShadow(juce::Colours::black.withAlpha(0.40f), 10, juce::Point<int>(1, 4));
        baseShadow.drawForPath(g, shadowPath);

        juce::ColourGradient nutGrad(nutTop.brighter(0.08f), 0.0f, cy - nutH * 0.5f,
                                     nutBottom.darker(0.05f), 0.0f, cy + nutH * 0.5f, false);
        juce::Path nut;
        nut.addEllipse(juce::Rectangle<float>(cx - nutR, cy - nutH * 0.5f, nutR * 2.0f, nutH));
        g.setGradientFill(nutGrad);
        g.fillPath(nut);
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawEllipse(cx - nutR, cy - nutH * 0.5f, nutR * 2.0f, nutH, 1.2f);

        const float washerY = cy - nutH * 0.6f;
        juce::ColourGradient washerGrad(chromeTop, 0.0f, washerY - 3.0f,
                                        chromeMid.darker(0.1f), 0.0f, washerY + 2.0f, false);
        juce::Path washer;
        washer.addEllipse(juce::Rectangle<float>(cx - nutR * 0.85f, washerY - 3.0f, nutR * 1.7f, 6.0f));
        g.setGradientFill(washerGrad);
        g.fillPath(washer);

        const float pivotX = cx;
        const float pivotY = cy - 10.0f;
        const float batLen = 36.0f;
        const float leanAngle = juce::degreesToRadians(5.0f);
        const float swingAngle = (animT - 0.5f) * juce::degreesToRadians(50.0f);

        const float ba = leanAngle + swingAngle;
        const float tipX = pivotX + batLen * std::sin(ba);
        const float tipY = pivotY - batLen * std::cos(ba);

        juce::Path bat;
        bat.startNewSubPath(pivotX - 5.5f, pivotY);
        bat.lineTo(pivotX + 5.5f, pivotY);
        bat.lineTo(tipX + 4.0f, tipY);
        bat.lineTo(tipX - 4.0f, tipY);
        bat.closeSubPath();

        juce::ColourGradient batGrad(chromeTop, pivotX - 6.0f, pivotY,
                                     chromeMid.darker(0.08f), pivotX + 6.0f, tipY, false);
        g.setGradientFill(batGrad);
        g.fillPath(bat);

        juce::Path tip;
        tip.addEllipse(juce::Rectangle<float>(tipX - 4.5f, tipY - 4.0f, 9.0f, 8.0f));
        juce::ColourGradient tipGrad(chromeTop, tipX - 4.5f, tipY - 4.0f,
                                     chromeMid, tipX + 4.5f, tipY + 4.0f, false);
        g.setGradientFill(tipGrad);
        g.fillPath(tip);

        g.setColour(juce::Colours::black.withAlpha(0.12f));
        g.drawLine(tipX - 4.5f, tipY, tipX + 4.5f, tipY, 0.8f);
        g.setColour(juce::Colours::white.withAlpha(0.25f));
        g.drawLine(tipX - 3.0f, tipY - 3.5f, tipX + 2.0f, tipY - 3.5f, 1.0f);

        g.setColour(juce::Colours::black.withAlpha(0.30f * (1.0f - animT)));
        g.drawLine(pivotX - 6.0f, pivotY + nutH * 0.7f, pivotX + 6.0f, pivotY + nutH * 0.7f, 2.0f);
    }
}
