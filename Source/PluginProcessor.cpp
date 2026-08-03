#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "dsp/Conditioning.h"
#include <cmath>

BM176AudioProcessor::BM176AudioProcessor()
    : juce::AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts(*this, nullptr, "BM176", createParameterLayout())
{
    pRatio         = apvts.getRawParameterValue("ratio");
    pInput         = apvts.getRawParameterValue("input");
    pInputVernier  = apvts.getRawParameterValue("inputVernier");
    pThreshold     = apvts.getRawParameterValue("threshold");
    pSidechain     = apvts.getRawParameterValue("sidechain");
    pAttack        = apvts.getRawParameterValue("attack");
    pRelease       = apvts.getRawParameterValue("release");
    pOutput        = apvts.getRawParameterValue("output");
    pOutputVernier = apvts.getRawParameterValue("outputVernier");
    pInterstage    = apvts.getRawParameterValue("interstage");
    pBypass        = apvts.getRawParameterValue("bypass");
    pCompressorOn  = apvts.getRawParameterValue("compressorOn");
    pPower         = apvts.getRawParameterValue("power");
}

juce::AudioProcessorValueTreeState::ParameterLayout BM176AudioProcessor::createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<AudioParameterChoice>("ratio", "Ratio",
        StringArray{"1.5:1", "2:1", "4:1", "8:1", "12:1"}, 2));

    layout.add(std::make_unique<AudioParameterFloat>("input", "Input",
        NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));

    layout.add(std::make_unique<AudioParameterFloat>("inputVernier", "Input Vernier",
        NormalisableRange<float>(-1.0f, 1.0f, 0.01f), -1.0f));

    layout.add(std::make_unique<AudioParameterFloat>("threshold", "Threshold",
        NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));

    layout.add(std::make_unique<AudioParameterChoice>("sidechain", "Sidechain HP",
        StringArray{"OFF", "45", "80", "120", "150", "200"}, 0));

    layout.add(std::make_unique<AudioParameterChoice>("meterMode", "Meter",
        StringArray{"IN", "GR", "OUT"}, 1));

    layout.add(std::make_unique<AudioParameterFloat>("attack", "Attack",
        NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));

    layout.add(std::make_unique<AudioParameterFloat>("release", "Release",
        NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));

    layout.add(std::make_unique<AudioParameterFloat>("output", "Output",
        NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));

    layout.add(std::make_unique<AudioParameterFloat>("outputVernier", "Output Vernier",
        NormalisableRange<float>(-1.0f, 1.0f, 0.01f), -1.0f));

    layout.add(std::make_unique<AudioParameterBool>("interstage", "Interstage", true));
    layout.add(std::make_unique<AudioParameterBool>("bypass", "Bypass", false));
    layout.add(std::make_unique<AudioParameterBool>("compressorOn", "Compressor", true));
    layout.add(std::make_unique<AudioParameterBool>("power", "Power", true));

    return layout;
}

const juce::String BM176AudioProcessor::getName() const { return "BM176"; }

bool BM176AudioProcessor::acceptsMidi() const { return false; }
bool BM176AudioProcessor::producesMidi() const { return false; }
double BM176AudioProcessor::getTailLengthSeconds() const { return 0.0; }

int BM176AudioProcessor::getNumPrograms() { return 1; }
int BM176AudioProcessor::getCurrentProgram() { return 0; }
void BM176AudioProcessor::setCurrentProgram(int) {}
const juce::String BM176AudioProcessor::getProgramName(int) { return {}; }
void BM176AudioProcessor::changeProgramName(int, const juce::String&) {}

void BM176AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    sampleRate_ = sampleRate;
    chain.prepare(sampleRate);
    sidechainHP.reset();
}

void BM176AudioProcessor::releaseResources()
{
    chain.reset();
    sidechainHP.reset();
}

bool BM176AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void BM176AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const float bypassVal   = pBypass->load();
    const float powerVal    = pPower->load();

    if (bypassVal >= 0.5f || powerVal < 0.5f)
        return;

    const float ratioVal        = pRatio->load();
    const float inputVal        = pInput->load();
    const float inputVernierVal = pInputVernier->load();
    const float thresholdVal    = pThreshold->load();
    const float sidechainVal    = pSidechain->load();
    const float attackVal       = pAttack->load();
    const float releaseVal      = pRelease->load();
    const float outputVal       = pOutput->load();
    const float outputVernierVal = pOutputVernier->load();
    const float interstageVal   = pInterstage->load();
    const float compressorOnVal = pCompressorOn->load();

    // Map ratio choice index to ratio value
    const float ratioMap[5] = { 1.5f, 2.0f, 4.0f, 8.0f, 12.0f };
    const int ratioIdx = juce::jlimit(0, 4, static_cast<int>(ratioVal));
    const float ratioDial = ratioMap[ratioIdx];

    // Conditioning: 7-vector for the neural model
    const auto cond = ua176::normalizeConditioning(
        ratioDial, inputVal, thresholdVal, attackVal, outputVal, releaseVal,
        interstageVal);

    // Compressor bypass (attack OFF switch maps to limiting_action)
    const bool compressorOn = compressorOnVal >= 0.5f;
    chain.setBypassed(!compressorOn);

    // Sidechain HP index + freq table
    const float hpFreqs[6] = { 0.0f, 45.0f, 80.0f, 120.0f, 150.0f, 200.0f };
    const int hpIdx = juce::jlimit(0, 5, static_cast<int>(sidechainVal));

    // Recomputed biquad coeffs on sidechain param change
    {
        static int lastHpIdx = -1;
        if (hpIdx != lastHpIdx)
        {
            lastHpIdx = hpIdx;
            if (hpIdx > 0)
                sidechainHP.computeCoeffs(ua176::BiquadType::HighPass, 0.0f,
                                          hpFreqs[hpIdx], 0.7071f, sampleRate_);
        }
    }

    // Process audio
    const int numSamples = buffer.getNumSamples();
    float* channelData = buffer.getWritePointer(0);

    // Sidechain HP pre-filter (affects both audio and detector; pragmatic until
    // VariMuDrcProcessor supports external sidechain input)
    if (hpIdx > 0)
    {
        for (int i = 0; i < numSamples; ++i)
            channelData[i] = sidechainHP.processSample(channelData[i]);
    }

    // Run the neural model chain
    chain.process(channelData, numSamples, cond);

    // Gain anchor — corrects the model chain output to match hardware throughput.
    // Measured 2026-08-03: chain_out = -26.56 dBFS RMS (-23.55 dB peak) at training ref
    // (in=5,out=5,vi=10,vo=0,compOFF). Hardware at same settings: -19.6 dBFS peak.
    // Anchor = -19.6 - (-23.55) = +3.95 dB.
    // TODO: retrain controllers with normalized NL, then remove this anchor.
    {
        constexpr float kGainAnchorDb = 3.95f;
        const float anchorLin = std::pow(10.0f, kGainAnchorDb / 20.0f);
        juce::FloatVectorOperations::multiply(channelData, anchorLin, numSamples);
    }

    // Vernier trim — delta from training references.
    // Training data: vernier_in=10 (5 o'clock, max), vernier_out=0 (7 o'clock, min).
    // BM176 verniers are -1 to 1; map to 0-10 dial space.
    // M5 calibration (2026-08-02): in={0:-2.39, 5:-1.34, 10:0.0}, out={0:0.0, 5:2.10, 10:3.80}
    const float viDial = (inputVernierVal + 1.0f) * 5.0f;
    const float voDial = (outputVernierVal + 1.0f) * 5.0f;
    auto vernierGain = [](float dial, float d0, float d5, float d10) -> float {
        if (dial <= 5.0f) return d0 + (d5 - d0) * (dial / 5.0f);
        return d5 + (d10 - d5) * ((dial - 5.0f) / 5.0f);
    };
    const float viDb = vernierGain(viDial, -2.39f, -1.34f, 0.0f)
                       - vernierGain(10.0f,  -2.39f, -1.34f, 0.0f);
    const float voDb = vernierGain(voDial, 0.0f,   2.10f, 3.80f)
                       - vernierGain(0.0f,   0.0f,  2.10f, 3.80f);
    const float vernierDb = viDb + voDb;
    if (std::abs(vernierDb) > 0.001f)
    {
        const float gainLin = std::pow(10.0f, vernierDb / 20.0f);
        juce::FloatVectorOperations::multiply(channelData, gainLin, numSamples);
    }

    // Duplicate mono result to output channels
    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom(ch, 0, buffer, 0, 0, numSamples);

    // Meter levels (RMS, ~1% of captured range above noise floor)
    {
        float sumSq = 0.0f;
        for (int i = 0; i < numSamples; ++i)
            sumSq += channelData[i] * channelData[i];
        const float rms = std::sqrt(sumSq / static_cast<float>(numSamples));
        const float rmsDb = 20.0f * std::log10(rms + 1e-10f);
        outputLevelDb.store(rmsDb);
    }
}

bool BM176AudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* BM176AudioProcessor::createEditor()
{
    return new BM176AudioProcessorEditor(*this);
}

void BM176AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BM176AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BM176AudioProcessor();
}
