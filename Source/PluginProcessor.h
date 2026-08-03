#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/GreyBoxChain.h"
#include "dsp/Biquad.h"

class BM176AudioProcessor : public juce::AudioProcessor
{
public:
    BM176AudioProcessor();
    ~BM176AudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    float getGainReductionDb() const noexcept { return chain.getGainReductionDb(); }
    float getInputLevelDb() const noexcept { return inputLevelDb.load(); }
    float getOutputLevelDb() const noexcept { return outputLevelDb.load(); }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    ua176::GreyBoxChain chain;
    ua176::Biquad sidechainHP;

    std::atomic<float> inputLevelDb{ -96.0f };
    std::atomic<float> outputLevelDb{ -96.0f };

    // Raw parameter pointers for fast audio-thread access
    std::atomic<float>* pRatio = nullptr;
    std::atomic<float>* pInput = nullptr;
    std::atomic<float>* pInputVernier = nullptr;
    std::atomic<float>* pThreshold = nullptr;
    std::atomic<float>* pSidechain = nullptr;
    std::atomic<float>* pAttack = nullptr;
    std::atomic<float>* pRelease = nullptr;
    std::atomic<float>* pOutput = nullptr;
    std::atomic<float>* pOutputVernier = nullptr;
    std::atomic<float>* pInterstage = nullptr;
    std::atomic<float>* pBypass = nullptr;
    std::atomic<float>* pCompressorOn = nullptr;
    std::atomic<float>* pPower = nullptr;

    double sampleRate_ = 48000.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BM176AudioProcessor)
};
