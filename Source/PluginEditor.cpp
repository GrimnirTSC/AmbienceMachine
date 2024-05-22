#include "PluginProcessor.h"
#include "PluginEditor.h"

AmbienceMachineAudioProcessorEditor::AmbienceMachineAudioProcessorEditor(AmbienceMachineAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    addAndMakeVisible(loadButton1);
    loadButton1.addListener(this);

    addAndMakeVisible(gainSliderAmbience);
    gainSliderAmbience.setRange(0.0, 1.0);
    gainSliderAmbience.setValue(0.5);
    gainSliderAmbience.addListener(this);

    addAndMakeVisible(loadButton2);
    loadButton2.addListener(this);

    addAndMakeVisible(gainSliderRain);
    gainSliderRain.setRange(0.0, 1.0);
    gainSliderRain.setValue(0.5);
    gainSliderRain.addListener(this);

    setSize(800, 600);
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
    loadButton1.setBounds(10, 10, getWidth() - 20, 30);
    gainSliderAmbience.setBounds(10, 50, getWidth() - 20, 30);
    loadButton2.setBounds(10, 80, getWidth() - 20, 30);
    gainSliderRain.setBounds(10, 120, getWidth() - 20, 30);
}

void AmbienceMachineAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &loadButton1)
    {
        juce::FileChooser chooser("Select an ambience file...");
        if (chooser.browseForFileToOpen())
        {
            auto file = chooser.getResult();
            audioProcessor.loadFile(file);
        }
    }
    if (button == &loadButton2)
    {
        juce::FileChooser chooser("Select a RainSound file...");
        if (chooser.browseForFileToOpen())
        {
            auto file = chooser.getResult();
            audioProcessor.loadFile(file);
        }
    }
}

void AmbienceMachineAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &gainSliderAmbience)
    {
        audioProcessor.setGainAmbience(slider->getValue());
    }
    if (slider == &gainSliderRain)
    {
        audioProcessor.setGainRain(slider->getValue());
    }
}