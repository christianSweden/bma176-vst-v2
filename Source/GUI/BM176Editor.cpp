#include "BM176Editor.h"
#include "BM176Geometry.h"

namespace bm176
{
    BM176Editor::BM176Editor()
    {
        addAndMakeVisible(panel);

        ratioKnob.setDiscrete(true, 5);
        ratioKnob.setAngleRange(-75.0f, 75.0f);
        ratioKnob.setValue(5.0f);
        addAndMakeVisible(ratioKnob);

        sidechainKnob.setDiscrete(true, 6);
        sidechainKnob.setAngleRange(-90.0f, 90.0f);
        sidechainKnob.setValue(0.0f);
        addAndMakeVisible(sidechainKnob);

        inputKnob.setIsBig(true);
        inputKnob.setDiscrete(true, 21);
        inputKnob.setValue(5.0f);
        addAndMakeVisible(inputKnob);

        meterKnob.setDiscrete(true, 3);
        meterKnob.setAngleRange(-60.0f, 60.0f);
        meterKnob.setValue(5.0f);
        addAndMakeVisible(meterKnob);

        vernierInKnob.setVernierMode(true);
        vernierInKnob.setValue(-1.0f);
        addAndMakeVisible(vernierInKnob);

        thresholdKnob.setValue(5.0f);
        addAndMakeVisible(thresholdKnob);

        addAndMakeVisible(vuMeter);

        interstageSwitch.setLabels("", "OUT", "IN");
        interstageSwitch.setState(true);
        addAndMakeVisible(interstageSwitch);

        attackOffSwitch.setLabels("", "OFF", "ON");
        attackOffSwitch.setState(true);
        addAndMakeVisible(attackOffSwitch);

        attackKnob.setValueRange(2.0f, 10.0f);
        attackKnob.setValue(5.0f);
        addAndMakeVisible(attackKnob);

        vernierOutKnob.setVernierMode(true);
        vernierOutKnob.setValue(-1.0f);
        addAndMakeVisible(vernierOutKnob);

        outputKnob.setIsBig(true);
        outputKnob.setDiscrete(true, 21);
        outputKnob.setValue(5.0f);
        addAndMakeVisible(outputKnob);

        releaseKnob.setValue(5.0f);
        addAndMakeVisible(releaseKnob);

        bypassSwitch.setLabels("", "IN", "BYPASS");
        bypassSwitch.setState(false);
        addAndMakeVisible(bypassSwitch);

        onLamp.setState(true);
        addAndMakeVisible(onLamp);

        powerSwitch.setLabels("", "ON", "OFF");
        powerSwitch.setState(false);
        powerSwitch.setCallback([this](bool on) { onLamp.setState(!on); });
        addAndMakeVisible(powerSwitch);

        addAndMakeVisible(inputJack);
        addAndMakeVisible(hiZJack);
        addAndMakeVisible(outputJack);

        setSize(DESIGN_WIDTH, DESIGN_HEIGHT);
        panel.toBack();
    }

    void BM176Editor::resized()
    {
        panel.setBounds(getLocalBounds());

        auto placeCircle = [&](juce::Component& c, float cx, float cy, float d)
        {
            const float margin = 8.0f;
            c.setBounds(juce::roundToInt(cx - d * 0.5f - margin),
                        juce::roundToInt(cy - d * 0.5f - margin),
                        juce::roundToInt(d + margin * 2.0f),
                        juce::roundToInt(d + margin * 2.0f));
        };
        auto placeToggle = [&](juce::Component& c, float cx, float cy)
        {
            c.setBounds(juce::roundToInt(cx - 30.0f), juce::roundToInt(cy - 12.0f - 48.0f), 60, 96);
        };

        placeCircle(ratioKnob,      196.0f, 103.0f, 82.0f);
        placeCircle(sidechainKnob,  196.0f, 264.0f, 82.0f);
        placeCircle(inputKnob,      376.0f, 171.0f, 158.0f);
        placeCircle(meterKnob,      567.0f,  98.0f, 82.0f);
        placeCircle(vernierInKnob,  567.0f, 253.0f, 82.0f);
        placeCircle(thresholdKnob,  679.0f, 166.0f, 82.0f);
        placeCircle(attackKnob,    1364.0f,  88.0f, 82.0f);
        placeCircle(vernierOutKnob,1364.0f, 253.0f, 82.0f);
        placeCircle(outputKnob,    1555.0f, 171.0f, 158.0f);
        placeCircle(releaseKnob,   1734.0f,  88.0f, 82.0f);

        auto placeSwitch = [&](juce::Component& c, float cx, float cy)
        {
            c.setBounds(juce::roundToInt(cx - 18.0f), juce::roundToInt(cy - 50.0f), 36, 100);
        };

        placeSwitch(interstageSwitch, 1253.0f, 176.0f);
        placeSwitch(attackOffSwitch,  1253.0f, 303.0f);
        bypassSwitch.setBounds(juce::roundToInt(1848.0f - 18.0f), juce::roundToInt(88.0f - 50.0f), 36, 100);
        placeSwitch(powerSwitch,      1848.0f, 274.0f);

        vuMeter.setBounds(juce::roundToInt(VU_BEZEL_X), juce::roundToInt(VU_BEZEL_Y),
                          juce::roundToInt(VU_BEZEL_W), juce::roundToInt(VU_BEZEL_H));

        placeCircle(onLamp,  1807.0f, 284.0f, 30.0f);
        placeCircle(inputJack,  90.0f,  77.0f, 46.0f);
        placeCircle(hiZJack,    90.0f, 171.0f, 46.0f);
        placeCircle(outputJack, 90.0f, 254.0f, 46.0f);
    }
}
