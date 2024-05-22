#pragma once

#include <JuceHeader.h>

class AmbienceMachineAudioProcessor : public juce::AudioProcessor
{
public:
    AmbienceMachineAudioProcessor();
    ~AmbienceMachineAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void loadFile(const juce::File& file);
    void setGainAmbience(float gain);
    void setGainRain(float gain);


private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::AudioProcessorValueTreeState parameters;
    juce::AudioParameterFloat* gainParameterAmbience;
    juce::AudioParameterFloat* gainParameterRain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AmbienceMachineAudioProcessor)
};