#include "PluginEditor.h"
#include "GUI/BM176Geometry.h"

BM176AudioProcessorEditor::BM176AudioProcessorEditor(BM176AudioProcessor&)
    : juce::AudioProcessorEditor(nullptr)
{
    setSize(bm176::DEFAULT_WINDOW_W, bm176::DEFAULT_WINDOW_H);
    setResizable(true, true);
    setResizeLimits(720, static_cast<int>(720.0 / bm176::DESIGN_ASPECT),
                    2560, static_cast<int>(2560.0 / bm176::DESIGN_ASPECT));
    addAndMakeVisible(editor);
}

void BM176AudioProcessorEditor::resized()
{
    const auto bounds = bm176::getDesignBounds(getWidth(), getHeight());
    editor.setTransform(bm176::getScaleTransform(getWidth(), getHeight()));
    editor.setBounds(bounds.toNearestInt());
}
