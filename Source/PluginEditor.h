#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "GUI/BM176Editor.h"

class BM176AudioProcessorEditor : public juce::AudioProcessorEditor,
                                  public juce::Timer
{
public:
    explicit BM176AudioProcessorEditor(BM176AudioProcessor&);
    ~BM176AudioProcessorEditor() override;
    void resized() override;
    void timerCallback() override;

private:
    BM176AudioProcessor& processorRef;
    bm176::BM176Editor editor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BM176AudioProcessorEditor)
};
