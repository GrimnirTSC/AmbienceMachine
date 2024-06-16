
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CustomLookAndFeel.h"
#include "VUMeter.h"

AmbienceMachineAudioProcessorEditor::AmbienceMachineAudioProcessorEditor(AmbienceMachineAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), customLookAndFeel(), meter(p.analysis)  
{
    CustomLookAndFeel::setDefaultLookAndFeel(&customLookAndFeel);

    addAndMakeVisible(loadButtonAmbience);
    loadButtonAmbience.addListener(this);

    addAndMakeVisible(gainSliderAmbience);
    gainSliderAmbience.setRange(0.0, 1.0);
    if (auto* gainParam = p.parameters.getRawParameterValue("gainAmbience"))
    {
        gainSliderAmbience.setValue(*gainParam); 
    }
    gainSliderAmbience.addListener(this);

    addAndMakeVisible(ambienceGainLabel);

    addAndMakeVisible(loadButtonRain);
    loadButtonRain.addListener(this);

    addAndMakeVisible(gainSliderRain);
    gainSliderRain.setRange(0.0, 1.0);
    gainSliderRain.addListener(this);
    gainSliderRain.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSliderRain.setTextBoxStyle(juce::Slider::NoTextBox, true, 50, 20);

    rainGainLabel.setText("Intensity Rain", juce::dontSendNotification);
    rainGainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(rainGainLabel);

    gainSliderAmbience.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSliderAmbience.setTextBoxStyle(juce::Slider::NoTextBox, true, 50, 20);
    ambienceGainLabel.setText("Gain Ambience", juce::dontSendNotification);
    ambienceGainLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(loadButtonOneshot);
    loadButtonOneshot.addListener(this);

    addAndMakeVisible(gainSliderOneshot);
    gainSliderOneshot.setRange(0.0, 1.0);
    gainSliderOneshot.addListener(this);
    gainSliderOneshot.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSliderOneshot.setTextBoxStyle(juce::Slider::NoTextBox, true, 50, 20);
    oneshotGainLabel.setText("Gain Oneshot", juce::dontSendNotification);
    oneshotGainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(oneshotGainLabel);

    addAndMakeVisible(FrequencySliderOneshot);
    FrequencySliderOneshot.setRange(0.0, 40.0);
    FrequencySliderOneshot.addListener(this);
    FrequencySliderOneshot.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    FrequencySliderOneshot.setTextBoxStyle(juce::Slider::NoTextBox, true, 50, 20);
    oneshotFreqLabel.setText("Frequency Oneshot", juce::dontSendNotification);
    oneshotFreqLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(oneshotFreqLabel);

    TitleLabel.setText("The Great Big AMBIENCE MACHINE", juce::dontSendNotification);
    TitleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(TitleLabel);

    addAndMakeVisible(meter);
    setOpaque(true);
    setWantsKeyboardFocus(false);


    setSize(600, 600);

}

AmbienceMachineAudioProcessorEditor::~AmbienceMachineAudioProcessorEditor()
{
}




void AmbienceMachineAudioProcessorEditor::paint(juce::Graphics& g)
{
    int editorWidth = getWidth();



    g.fillAll(juce::Colour(245, 240, 235));
    g.fillRect(240, 380, 400, 250);


    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);


    g.drawRect(0, 0, editorWidth, 190, 2);
    g.drawRect(0, 190, editorWidth - 300, 190, 2);
    g.drawRect(300, 190, editorWidth - 300, 190, 2);
    g.drawRect(0, 380, editorWidth, 220, 2);


}

void AmbienceMachineAudioProcessorEditor::resized()
{
    loadButtonAmbience.setBounds(70, 50, getWidth() - 150, 30);
    gainSliderAmbience.setBounds(70, 90, getWidth() - 150, 90);
    ambienceGainLabel.setBounds(140, 120, getWidth() - 500, 30);

    loadButtonRain.setBounds(10, 220, getWidth() - 350, 30);
    gainSliderRain.setBounds(10, 260, getWidth() - 350, 90);
    rainGainLabel.setBounds(85, 340 , getWidth() - 500, 30);


    loadButtonOneshot.setBounds(330, 220, getWidth() - 350, 30);
    FrequencySliderOneshot.setBounds(420, 280, getWidth() - 400, 60);
    gainSliderOneshot.setBounds(310, 280, getWidth() - 400, 60);
    oneshotFreqLabel.setBounds(475, 340, getWidth() - 500, 30);
    oneshotGainLabel.setBounds(355, 340, getWidth() - 500, 30);
    meter.setBounds(0, 380, 100, 220);
    
    TitleLabel.setBounds(140, 360, 400, 250);
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
    else if (slider == &gainSliderRain)
    {
        DBG("Rain gain slider value changed: " + juce::String(slider->getValue()));
        audioProcessor.setGainRain(slider->getValue(), slider->getValue());
    }
    else if (slider == &FrequencySliderOneshot)
    {
        DBG("Frequency slider value changed: " + juce::String(slider->getValue()));
        audioProcessor.setFrequencyOneshot(slider->getValue());
    }
    else if (slider == &gainSliderOneshot)
    {
        DBG("oneshot gain slider value changed: " + juce::String(slider->getValue()));
        audioProcessor.setGainOneshot(slider->getValue());
    }
}
