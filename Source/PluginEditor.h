#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "GUI/BM176Editor.h"

class BM176AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit BM176AudioProcessorEditor(BM176AudioProcessor&);
    ~BM176AudioProcessorEditor() override = default;
    void resized() override;

private:
    bm176::BM176Editor editor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BM176AudioProcessorEditor)
};
