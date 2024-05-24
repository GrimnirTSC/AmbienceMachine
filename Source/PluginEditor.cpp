#include "PluginProcessor.h"
#include "PluginEditor.h"

AmbienceMachineAudioProcessorEditor::AmbienceMachineAudioProcessorEditor(AmbienceMachineAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    addAndMakeVisible(loadButtonAmbience);
    loadButtonAmbience.addListener(this);

    addAndMakeVisible(gainSliderAmbience);
    gainSliderAmbience.setRange(0.0, 1.0);
    gainSliderAmbience.setValue(0.5);
    gainSliderAmbience.addListener(this);

    addAndMakeVisible(loadButtonRain);
    loadButtonRain.addListener(this);

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
    loadButtonAmbience.setBounds(10, 10, getWidth() - 20, 30);
    gainSliderAmbience.setBounds(10, 50, getWidth() - 20, 30);
    loadButtonRain.setBounds(10, 90, getWidth() - 20, 30);
    gainSliderRain.setBounds(10, 130, getWidth() - 20, 30);
}

void AmbienceMachineAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &loadButtonAmbience)
    {
        juce::FileChooser chooser("Select an ambience file...");
        if (chooser.browseForFileToOpen())
        {
            auto file = chooser.getResult();
            DBG("Ambience file selected: " + file.getFullPathName());
            audioProcessor.loadAmbienceFile(file);
        }
    }
    else if (button == &loadButtonRain)
    {
        juce::FileChooser chooser("Select a rain sound file...");
        if (chooser.browseForFileToOpen())
        {
            auto file = chooser.getResult();
            DBG("Rain file selected: " + file.getFullPathName());
            audioProcessor.loadRainFile(file);
        }
    }
}

void AmbienceMachineAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &gainSliderAmbience)
    {
        DBG("Ambience gain slider value changed: " + juce::String(slider->getValue()));
        audioProcessor.setGainAmbience(slider->getValue());
    }
    else if (slider == &gainSliderRain)
    {
        DBG("Rain gain slider value changed: " + juce::String(slider->getValue()));
        audioProcessor.setGainRain(slider->getValue());
    }
}