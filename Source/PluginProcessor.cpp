#include "PluginProcessor.h"
#include "PluginEditor.h"

BM176AudioProcessor::BM176AudioProcessor()
    : juce::AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
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

void BM176AudioProcessor::getStateInformation(juce::MemoryBlock&) {}
void BM176AudioProcessor::setStateInformation(const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BM176AudioProcessor();
}
