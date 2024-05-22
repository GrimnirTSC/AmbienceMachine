#include "PluginProcessor.h"
#include "PluginEditor.h"

AmbienceMachineAudioProcessorEditor::AmbienceMachineAudioProcessorEditor(AmbienceMachineAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    addAndMakeVisible(loadButton);
    loadButton.addListener(this);

    addAndMakeVisible(gainSlider);
    gainSlider.setRange(0.0, 1.0);
    gainSlider.setValue(0.5);
    gainSlider.addListener(this);

    setSize(400, 300);
}

AmbienceMachineAudioProcessorEditor::~AmbienceMachineAudioProcessorEditor()
{
}

void AmbienceMachineAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void AmbienceMachineAudioProcessorEditor::resized()
{
    loadButton.setBounds(10, 10, getWidth() - 20, 30);
    gainSlider.setBounds(10, 50, getWidth() - 20, 30);
}

void AmbienceMachineAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        juce::FileChooser chooser("Select an ambience file...");
        if (chooser.browseForFileToOpen())
        {
            auto file = chooser.getResult();
            audioProcessor.loadFile(file);
        }
    }
}

void AmbienceMachineAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &gainSlider)
    {
        audioProcessor.setGain(slider->getValue());
    }
}