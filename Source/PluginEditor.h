#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

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

    juce::TextButton loadButtonAmbience{ "Load Ambience" };
    juce::TextButton loadButtonRain{ "Load Rain" };

    juce::Slider gainSliderAmbience;
    juce::Slider gainSliderRain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AmbienceMachineAudioProcessorEditor)
};