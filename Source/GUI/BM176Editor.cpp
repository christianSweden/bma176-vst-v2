#include "BM176Editor.h"
#include "BM176Geometry.h"
#include <cmath>

namespace bm176
{
    BM176Editor::BM176Editor(juce::AudioProcessorValueTreeState& apvtsRef)
        : apvts(apvtsRef)
    {
        addAndMakeVisible(panel);

        // === Ratio (discrete, 5 positions) ===
        ratioKnob.setDiscrete(true, 5);
        ratioKnob.setAngleRange(-60.0f, 60.0f);
        addAndMakeVisible(ratioKnob);

        // === Sidechain HP (discrete, 6 positions) ===
        sidechainKnob.setDiscrete(true, 6);
        sidechainKnob.setAngleRange(-75.0f, 75.0f);
        addAndMakeVisible(sidechainKnob);

        // === Input (big, 21 detents) ===
        inputKnob.setIsBig(true);
        inputKnob.setDiscrete(true, 21);
        addAndMakeVisible(inputKnob);

        // === Meter mode (discrete, 3 positions) ===
        meterKnob.setDiscrete(true, 3);
        meterKnob.setAngleRange(-45.0f, 45.0f);
        addAndMakeVisible(meterKnob);

        // === Vernier In (-1 to 1, centre detent) ===
        vernierInKnob.setVernierMode(true);
        addAndMakeVisible(vernierInKnob);

        // === Threshold ===
        addAndMakeVisible(thresholdKnob);

        addAndMakeVisible(vuMeter);

        // === Interstage switch ===
        interstageSwitch.setLabels("OUT", "", "IN");
        addAndMakeVisible(interstageSwitch);

        // === Attack OFF switch (maps to compressorOn) ===
        attackOffSwitch.setLabels("", "OFF", "ON");
        addAndMakeVisible(attackOffSwitch);

        // === Attack (2-10 range, matches panel 2-8 + OFF switch) ===
        attackKnob.setValueRange(2.0f, 10.0f);
        addAndMakeVisible(attackKnob);

        // === Vernier Out (-1 to 1, centre detent) ===
        vernierOutKnob.setVernierMode(true);
        addAndMakeVisible(vernierOutKnob);

        // === Output (big, 21 detents) ===
        outputKnob.setIsBig(true);
        outputKnob.setDiscrete(true, 21);
        addAndMakeVisible(outputKnob);

        // === Release ===
        addAndMakeVisible(releaseKnob);

        // === Bypass switch ===
        bypassSwitch.setLabels("", "IN", "BYPASS");
        addAndMakeVisible(bypassSwitch);

        // === Power switch (decorative + disables audio when off) ===
        onLamp.setState(true);
        addAndMakeVisible(onLamp);

        powerSwitch.setLabels("", "ON", "OFF");
        powerSwitch.onStateVisual = [this](bool on) { onLamp.setState(!on); };
        addAndMakeVisible(powerSwitch);

        addAndMakeVisible(inputJack);
        addAndMakeVisible(hiZJack);
        addAndMakeVisible(outputJack);

        pMeterMode = apvts.getRawParameterValue("meterMode");

        setSize(DESIGN_WIDTH, DESIGN_HEIGHT);
        panel.toBack();

        // Constructed last, once every knob above is fully configured: each binder's
        // constructor fires an initial host->GUI update synchronously, and BMKnob::setValue()
        // needs isVernier/isDiscrete/minVal/maxVal to already reflect their final settings.
        ratioBinder = std::make_unique<BMKnobBinder>(apvtsRef, "ratio", ratioKnob,
            [](float v) { return std::round(v / 2.5f); },
            [](float v) { return v * 2.5f; });
        sidechainBinder = std::make_unique<BMKnobBinder>(apvtsRef, "sidechain", sidechainKnob,
            [](float v) { return std::round(v / 2.0f); },
            [](float v) { return v * 2.0f; });
        inputBinder = std::make_unique<BMKnobBinder>(apvtsRef, "input", inputKnob);
        meterBinder = std::make_unique<BMKnobBinder>(apvtsRef, "meterMode", meterKnob,
            [](float v) { return std::round(v / 5.0f); },
            [](float v) { return v * 5.0f; });
        vernierInBinder = std::make_unique<BMKnobBinder>(apvtsRef, "inputVernier", vernierInKnob);
        thresholdBinder = std::make_unique<BMKnobBinder>(apvtsRef, "threshold", thresholdKnob);
        attackBinder = std::make_unique<BMKnobBinder>(apvtsRef, "attack", attackKnob);
        vernierOutBinder = std::make_unique<BMKnobBinder>(apvtsRef, "outputVernier", vernierOutKnob);
        outputBinder = std::make_unique<BMKnobBinder>(apvtsRef, "output", outputKnob);
        releaseBinder = std::make_unique<BMKnobBinder>(apvtsRef, "release", releaseKnob);

        interstageBinder = std::make_unique<BMSwitchBinder>(apvtsRef, "interstage", interstageSwitch);
        attackOffBinder  = std::make_unique<BMSwitchBinder>(apvtsRef, "compressorOn", attackOffSwitch);
        bypassBinder     = std::make_unique<BMSwitchBinder>(apvtsRef, "bypass", bypassSwitch);
        powerBinder      = std::make_unique<BMSwitchBinder>(apvtsRef, "power", powerSwitch, true);
    }

    BM176Editor::~BM176Editor() = default;

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
        const int mode = juce::jlimit(0, 2, juce::roundToInt(pMeterMode->load()));

        float dbVal = -96.0f;
        switch (mode)
        {
            case 0: if (getInputLevelFn)    dbVal = getInputLevelFn();    break;
            case 1: if (getGainReductionFn) dbVal = getGainReductionFn(); break;
            case 2: if (getOutputLevelFn)   dbVal = getOutputLevelFn();   break;
        }

        vuMeter.setMode(mode == 1);
        vuMeter.setTargetDB(dbVal);
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
