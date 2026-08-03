#include "BM176Editor.h"
#include "BM176Geometry.h"

namespace bm176
{
    BM176Editor::BM176Editor(juce::AudioProcessorValueTreeState& apvtsRef)
        : apvts(apvtsRef)
    {
        addAndMakeVisible(panel);

        // === Ratio (discrete, 5 positions) ===
        ratioKnob.setDiscrete(true, 5);
        ratioKnob.setAngleRange(-75.0f, 75.0f);
        ratioKnob.setValue(5.0f);
        ratioKnob.onValueChange = [this](float v) {
            const int idx = juce::roundToInt(v / 2.5f);
            apvts.getParameter("ratio")->setValueNotifyingHost(idx / 4.0f);
        };
        addAndMakeVisible(ratioKnob);

        // === Sidechain HP (discrete, 6 positions) ===
        sidechainKnob.setDiscrete(true, 6);
        sidechainKnob.setAngleRange(-90.0f, 90.0f);
        sidechainKnob.setValue(0.0f);
        sidechainKnob.onValueChange = [this](float v) {
            const int idx = juce::roundToInt(v / 2.0f);
            apvts.getParameter("sidechain")->setValueNotifyingHost(idx / 5.0f);
        };
        addAndMakeVisible(sidechainKnob);

        // === Input (big, 21 detents) ===
        inputKnob.setIsBig(true);
        inputKnob.setDiscrete(true, 21);
        inputKnob.setValue(5.0f);
        inputKnob.onValueChange = [this](float v) {
            apvts.getParameter("input")->setValueNotifyingHost(v / 10.0f);
        };
        addAndMakeVisible(inputKnob);

        // === Meter mode (discrete, 3 positions) ===
        meterKnob.setDiscrete(true, 3);
        meterKnob.setAngleRange(-60.0f, 60.0f);
        meterKnob.setValue(5.0f);
        meterKnob.onValueChange = [this](float v) {
            const int idx = juce::roundToInt(v / 5.0f);
            meterModeIdx = juce::jlimit(0, 2, idx);
            apvts.getParameter("meterMode")->setValueNotifyingHost(idx / 2.0f);
        };
        addAndMakeVisible(meterKnob);

        // === Vernier In (-1 to 1, centre detent) ===
        vernierInKnob.setVernierMode(true);
        vernierInKnob.setValue(-1.0f);
        vernierInKnob.onValueChange = [this](float v) {
            apvts.getParameter("inputVernier")->setValueNotifyingHost((v + 1.0f) / 2.0f);
        };
        addAndMakeVisible(vernierInKnob);

        // === Threshold ===
        thresholdKnob.setValue(5.0f);
        thresholdKnob.onValueChange = [this](float v) {
            apvts.getParameter("threshold")->setValueNotifyingHost(v / 10.0f);
        };
        addAndMakeVisible(thresholdKnob);

        addAndMakeVisible(vuMeter);

        // === Interstage switch ===
        interstageSwitch.setLabels("", "OUT", "IN");
        interstageSwitch.setState(true);
        interstageSwitch.setCallback([this](bool on) {
            apvts.getParameter("interstage")->beginChangeGesture();
            apvts.getParameter("interstage")->setValueNotifyingHost(on ? 1.0f : 0.0f);
            apvts.getParameter("interstage")->endChangeGesture();
        });
        addAndMakeVisible(interstageSwitch);

        // === Attack OFF switch (maps to compressorOn) ===
        attackOffSwitch.setLabels("", "OFF", "ON");
        attackOffSwitch.setState(true);
        attackOffSwitch.setCallback([this](bool on) {
            apvts.getParameter("compressorOn")->beginChangeGesture();
            apvts.getParameter("compressorOn")->setValueNotifyingHost(on ? 1.0f : 0.0f);
            apvts.getParameter("compressorOn")->endChangeGesture();
        });
        addAndMakeVisible(attackOffSwitch);

        // === Attack (2-10 range, matches panel 2-8 + OFF switch) ===
        attackKnob.setValueRange(2.0f, 10.0f);
        attackKnob.setValue(5.0f);
        attackKnob.onValueChange = [this](float v) {
            apvts.getParameter("attack")->setValueNotifyingHost(v / 10.0f);
        };
        addAndMakeVisible(attackKnob);

        // === Vernier Out (-1 to 1, centre detent) ===
        vernierOutKnob.setVernierMode(true);
        vernierOutKnob.setValue(-1.0f);
        vernierOutKnob.onValueChange = [this](float v) {
            apvts.getParameter("outputVernier")->setValueNotifyingHost((v + 1.0f) / 2.0f);
        };
        addAndMakeVisible(vernierOutKnob);

        // === Output (big, 21 detents) ===
        outputKnob.setIsBig(true);
        outputKnob.setDiscrete(true, 21);
        outputKnob.setValue(5.0f);
        outputKnob.onValueChange = [this](float v) {
            apvts.getParameter("output")->setValueNotifyingHost(v / 10.0f);
        };
        addAndMakeVisible(outputKnob);

        // === Release ===
        releaseKnob.setValue(5.0f);
        releaseKnob.onValueChange = [this](float v) {
            apvts.getParameter("release")->setValueNotifyingHost(v / 10.0f);
        };
        addAndMakeVisible(releaseKnob);

        // === Bypass switch ===
        bypassSwitch.setLabels("", "IN", "BYPASS");
        bypassSwitch.setState(false);
        bypassSwitch.setCallback([this](bool on) {
            apvts.getParameter("bypass")->beginChangeGesture();
            apvts.getParameter("bypass")->setValueNotifyingHost(on ? 1.0f : 0.0f);
            apvts.getParameter("bypass")->endChangeGesture();
        });
        addAndMakeVisible(bypassSwitch);

        // === Power switch (decorative + disables audio when off) ===
        onLamp.setState(true);
        addAndMakeVisible(onLamp);

        powerSwitch.setLabels("", "ON", "OFF");
        powerSwitch.setState(false);
        powerSwitch.setCallback([this](bool on) {
            onLamp.setState(!on);
            apvts.getParameter("power")->beginChangeGesture();
            apvts.getParameter("power")->setValueNotifyingHost(on ? 0.0f : 1.0f);
            apvts.getParameter("power")->endChangeGesture();
        });
        addAndMakeVisible(powerSwitch);

        addAndMakeVisible(inputJack);
        addAndMakeVisible(hiZJack);
        addAndMakeVisible(outputJack);

        setSize(DESIGN_WIDTH, DESIGN_HEIGHT);
        panel.toBack();

        apvts.addParameterListener("bypass", this);
        apvts.addParameterListener("power", this);
        apvts.addParameterListener("interstage", this);
        apvts.addParameterListener("compressorOn", this);
        apvts.addParameterListener("meterMode", this);
    }

    BM176Editor::~BM176Editor()
    {
        apvts.removeParameterListener("bypass", this);
        apvts.removeParameterListener("power", this);
        apvts.removeParameterListener("interstage", this);
        apvts.removeParameterListener("compressorOn", this);
        apvts.removeParameterListener("meterMode", this);
    }

    void BM176Editor::setMeterSources(MeterSource gainReduction,
                                       MeterSource inputLevel,
                                       MeterSource outputLevel)
    {
        getGainReductionFn = std::move(gainReduction);
        getInputLevelFn = std::move(inputLevel);
        getOutputLevelFn = std::move(outputLevel);
    }

    void BM176Editor::timerCallback()
    {
        float dbVal = -96.0f;
        switch (meterModeIdx)
        {
            case 0: if (getInputLevelFn)  dbVal = getInputLevelFn(); break;
            case 1: if (getGainReductionFn) dbVal = getGainReductionFn(); break;
            case 2: if (getOutputLevelFn) dbVal = getOutputLevelFn(); break;
        }
        vuMeter.setTargetDB(dbVal);
    }

    void BM176Editor::parameterChanged(const juce::String& parameterID, float newValue)
    {
        if (parameterID == "bypass")
            bypassSwitch.setState(newValue >= 0.5f);
        else if (parameterID == "power")
            powerSwitch.setState(newValue < 0.5f);
        else if (parameterID == "interstage")
            interstageSwitch.setState(newValue >= 0.5f);
        else if (parameterID == "compressorOn")
            attackOffSwitch.setState(newValue >= 0.5f);
        else if (parameterID == "meterMode")
        {
            const int idx = juce::jlimit(0, 2, juce::roundToInt(newValue));
            meterModeIdx = idx;
        }
    }

    void BM176Editor::resized()
    {
        panel.setBounds(getLocalBounds());

        auto placeCircle = [&](juce::Component& c, float cx, float cy, float d)
        {
            c.setBounds(juce::roundToInt(cx - d * 0.5f), juce::roundToInt(cy - d * 0.5f),
                        juce::roundToInt(d), juce::roundToInt(d));
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
