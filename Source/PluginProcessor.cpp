#include "PluginProcessor.h"
#include "PluginEditor.h"

BM176AudioProcessor::BM176AudioProcessor()
    : juce::AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts(*this, nullptr, "BM176", createParameterLayout())
{
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
        NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));

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
        NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));

    layout.add(std::make_unique<AudioParameterBool>("interstage", "Interstage", true));
    layout.add(std::make_unique<AudioParameterBool>("bypass", "Bypass", false));
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

void BM176AudioProcessor::prepareToPlay(double, int) {}
void BM176AudioProcessor::releaseResources() {}

bool BM176AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void BM176AudioProcessor::processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) {}

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
