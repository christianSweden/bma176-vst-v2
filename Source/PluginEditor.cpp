#include "PluginEditor.h"
#include "GUI/BM176Geometry.h"

BM176AudioProcessorEditor::BM176AudioProcessorEditor(BM176AudioProcessor& p)
    : juce::AudioProcessorEditor(&p)
{
    const float aspect = bm176::DESIGN_WIDTH / static_cast<float>(bm176::DESIGN_HEIGHT);
    setResizable(true, true);
    setResizeLimits(720, juce::roundToInt(720.0f / aspect),
                    2560, juce::roundToInt(2560.0f / aspect));
    getConstrainer()->setFixedAspectRatio(aspect);
    setSize(bm176::DEFAULT_WINDOW_W, bm176::DEFAULT_WINDOW_H);
    addAndMakeVisible(editor);
}

void BM176AudioProcessorEditor::resized()
{
    const float sw = static_cast<float>(getWidth())  / static_cast<float>(bm176::DESIGN_WIDTH);
    const float sh = static_cast<float>(getHeight()) / static_cast<float>(bm176::DESIGN_HEIGHT);
    const float scale = juce::jmin(sw, sh);
    const float w = static_cast<float>(bm176::DESIGN_WIDTH)  * scale;
    const float h = static_cast<float>(bm176::DESIGN_HEIGHT) * scale;
    const float ox = (static_cast<float>(getWidth())  - w) * 0.5f;
    const float oy = (static_cast<float>(getHeight()) - h) * 0.5f;

    editor.setTransform(juce::AffineTransform::scale(scale));
    editor.setBounds(juce::roundToInt(ox), juce::roundToInt(oy),
                     juce::roundToInt(w), juce::roundToInt(h));
}
