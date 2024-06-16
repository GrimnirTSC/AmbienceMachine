#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h" // Include your custom look-and-feel header here
#include "VUMeter.h"

class AmbienceMachineAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Button::Listener,
    public juce::Slider::Listener
{
public:
    AmbienceMachineAudioProcessorEditor(AmbienceMachineAudioProcessor&);
    ~AmbienceMachineAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;


private:
    AmbienceMachineAudioProcessor& audioProcessor;
    CustomLookAndFeel customLookAndFeel; // Your custom look-and-feel object

    juce::TextButton loadButtonAmbience{ "Load Ambience" };
    juce::TextButton loadButtonRain{ "Load Rain" };
    juce::TextButton loadButtonOneshot{ "Load Oneshot" };

    juce::Slider gainSliderAmbience;
    juce::Slider gainSliderRain;
    juce::Slider gainSliderOneshot;
    juce::Slider FrequencySliderOneshot;

    juce::Label  ambienceGainLabel;
    juce::Label  rainGainLabel;
    juce::Label  oneshotGainLabel;

    juce::Label  oneshotFreqLabel;

    juce::Label  TitleLabel;

    VUMeter meter;




    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AmbienceMachineAudioProcessorEditor)
};
