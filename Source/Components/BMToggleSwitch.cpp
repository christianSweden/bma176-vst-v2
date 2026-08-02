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
        }
    }

    bool BMToggleSwitch::getState() const { return state; }

    void BMToggleSwitch::mouseDown(const juce::MouseEvent&)
    {
        setState(!state);
    }

    void BMToggleSwitch::timerCallback()
    {
        constexpr float step = 1.0f / 60.0f / animDuration;
        if (animT < animTarget - step)
            animT += step;
        else if (animT > animTarget + step)
            animT -= step;
        else if (animT != animTarget)
            animT = animTarget;
        else
            return;
        repaint();
    }

    void BMToggleSwitch::resized() {}

    void BMToggleSwitch::paint(juce::Graphics& g)
    {
        const float cx = getWidth() * 0.5f;
        const float nutCy = getHeight() * 0.5f;
        const float nutFlats = 34.0f;
        const float nutH = 20.0f;

        juce::Path shadowPath;
        shadowPath.addEllipse(juce::Rectangle<float>(cx - nutFlats * 0.6f, nutCy - nutFlats * 0.4f,
            nutFlats * 1.2f, nutFlats * 0.8f));
        juce::DropShadow ds(juce::Colours::black.withAlpha(0.45f), 10, juce::Point<int>(0, 5));
        ds.drawForPath(g, shadowPath);

        juce::ColourGradient nutGrad(
            nutTop, cx - nutFlats * 0.25f, nutCy - nutH * 0.5f,
            nutBottom, cx + nutFlats * 0.25f, nutCy + nutH * 0.5f, false);
        juce::Path nut;
        const float nutR = nutFlats * 0.65f;
        nut.addEllipse(juce::Rectangle<float>(cx - nutR, nutCy - nutH * 0.35f, nutR * 2.0f, nutH * 0.8f));
        g.setGradientFill(nutGrad);
        g.fillPath(nut);
        g.setColour(juce::Colours::black);
        g.drawEllipse(cx - nutR, nutCy - nutH * 0.35f, nutR * 2.0f, nutH * 0.8f, 1.0f);

        const float collarW = 18.0f;
        const float collarH = 8.0f;
        const float collarY = -9.0f;
        juce::ColourGradient collarGrad(
            chromeTop, cx - collarW * 0.3f, nutCy + collarY - collarH * 0.5f,
            chromeMid, cx + collarW * 0.3f, nutCy + collarY + collarH * 0.5f, false);
        juce::Path collar;
        collar.addEllipse(juce::Rectangle<float>(cx - collarW * 0.5f, nutCy + collarY - collarH * 0.5f,
            collarW, collarH));
        g.setGradientFill(collarGrad);
        g.fillPath(collar);

        const float pivotX = cx;
        const float pivotY = nutCy - 12.0f;

        const float upLen   = 34.0f;
        const float downLen = 28.0f;
        const float batLen  = downLen + (upLen - downLen) * animT;
        const float leanAngle = juce::degreesToRadians(6.0f);

        const float tipX = pivotX + batLen * std::sin(leanAngle);
        const float tipY = pivotY - batLen * std::cos(leanAngle);

        juce::Path bat;
        bat.startNewSubPath(pivotX - 6.0f, pivotY);
        bat.lineTo(pivotX + 6.0f, pivotY);
        bat.lineTo(tipX + 4.5f, tipY);
        bat.lineTo(tipX - 4.5f, tipY);
        bat.closeSubPath();

        juce::ColourGradient batGrad(chromeTop, pivotX - 6.0f, pivotY,
                                     chromeBottom, pivotX + 6.0f, tipY, false);
        g.setGradientFill(batGrad);
        g.fillPath(bat);

        juce::Path tip;
        tip.addEllipse(juce::Rectangle<float>(tipX - 4.5f, tipY - 4.5f, 9.0f, 9.0f));
        juce::ColourGradient tipGrad(chromeTop, tipX - 4.5f, tipY - 4.5f,
                                     chromeMid, tipX + 4.5f, tipY + 4.5f, false);
        g.setGradientFill(tipGrad);
        g.fillPath(tip);

        if (animT < 0.99f)
        {
            g.setColour(juce::Colours::black.withAlpha(0.35f * (1.0f - animT)));
            g.drawLine(pivotX - 7.0f, pivotY + 7.0f, pivotX + 7.0f, pivotY + 7.0f, 3.0f);
        }
    }
}
