#include "PluginProcessor.h"
#include "PluginEditor.h"

AmbienceMachineAudioProcessorEditor::AmbienceMachineAudioProcessorEditor(AmbienceMachineAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{

    addAndMakeVisible(loadButtonAmbience);
    loadButtonAmbience.addListener(this);

    addAndMakeVisible(gainSliderAmbience);
    gainSliderAmbience.setRange(0.0, 1.0);
    if (auto* gainParam = p.parameters.getRawParameterValue("gainAmbience"))
    {
        gainSliderAmbience.setValue(*gainParam); // Set slider value to the parameter's current value
    }
    gainSliderAmbience.addListener(this);

    addAndMakeVisible(loadButtonRain);
    loadButtonRain.addListener(this);

    addAndMakeVisible(gainSliderRain);
    gainSliderRain.setRange(0.0, 1.0);
    //gainSliderRain.setValue(audioProcessor.parameter.getParameter("gainRain")->getValue());
    gainSliderRain.addListener(this);

    addAndMakeVisible(loadButtonOneshot);
    loadButtonOneshot.addListener(this); // Correct listener added here

    addAndMakeVisible(gainSliderOneshot);
    gainSliderOneshot.setRange(0.0, 1.0);
    gainSliderOneshot.addListener(this);

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
    loadButtonOneshot.setBounds(10, 170, getWidth() - 20, 30);
    gainSliderOneshot.setBounds(10, 210, getWidth() - 20, 30);
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
    if (button == &loadButtonRain)
    {
        juce::FileChooser chooser("Select a rain sound file...");
        if (chooser.browseForFileToOpen())
        {
            auto file = chooser.getResult();
            DBG("Rain file selected: " + file.getFullPathName());
            audioProcessor.loadRainFile(file);
        }
    }
    else if (button == &loadButtonOneshot)
    {
        juce::FileChooser chooser("Select a oneshot sound file...");
        if (chooser.browseForFileToOpen())
        {
            auto file = chooser.getResult();
            DBG("oneshot file selected: " + file.getFullPathName());
            audioProcessor.loadOneshotFile(file);
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
    if (slider == &gainSliderRain)
    {
        DBG("Rain gain slider value changed: " + juce::String(slider->getValue()));
        audioProcessor.setGainRain(slider->getValue(), slider->getValue());
    }
    else if (slider == &gainSliderOneshot)
    {
        DBG("oneshot gain slider value changed: " + juce::String(slider->getValue()));
        audioProcessor.setGainOneshot(slider->getValue());
    }
}