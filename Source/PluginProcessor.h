#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>


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

    void loadAmbienceFile(const juce::File& file);
    void loadRainFile(const juce::File& file);
    void setGainAmbience(float gain);
    void setGainRain(float gain,float highpass);
    const int getParameterIDGainAmbience() const;
    juce::AudioProcessorValueTreeState parameters;

private:
    //juce::dsp::ProcessorChain<juce::dsp::Gain<float>, juce::dsp::IIR::Filter<float>> highPassChain;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSourceAmbience;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSourceRain;
    juce::AudioTransportSource transportSourceAmbience;
    juce::AudioTransportSource transportSourceRain;
    juce::AudioParameterFloat* gainParameterAmbience;
    juce::AudioParameterFloat* gainParameterRain;
    juce::AudioParameterFloat* highpassParameterRain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AmbienceMachineAudioProcessor)
};