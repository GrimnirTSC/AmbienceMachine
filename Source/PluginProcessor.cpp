#include "PluginProcessor.h"
#include "PluginEditor.h"

AmbienceMachineAudioProcessor::AmbienceMachineAudioProcessor()
    : parameters(*this, nullptr, "Parameters", { std::make_unique<juce::AudioParameterFloat>("gain", "Gain", 0.0f, 1.0f, 0.5f) })
{
    formatManager.registerBasicFormats();
}

AmbienceMachineAudioProcessor::~AmbienceMachineAudioProcessor() {}

const juce::String AmbienceMachineAudioProcessor::getName() const { return JucePlugin_Name; }

bool AmbienceMachineAudioProcessor::acceptsMidi() const
{
    return false;
}

bool AmbienceMachineAudioProcessor::producesMidi() const
{
    return false;
}

bool AmbienceMachineAudioProcessor::isMidiEffect() const
{
    return false;
}

double AmbienceMachineAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AmbienceMachineAudioProcessor::getNumPrograms()
{
    return 0;
}

int AmbienceMachineAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AmbienceMachineAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String AmbienceMachineAudioProcessor::getProgramName(int index)
{
    return juce::String();
}

void AmbienceMachineAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void AmbienceMachineAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
}

void AmbienceMachineAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
}

void AmbienceMachineAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
}

void AmbienceMachineAudioProcessor::releaseResources() { transportSource.releaseResources(); }

void AmbienceMachineAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    if (readerSource.get() == nullptr)
        return;

    transportSource.getNextAudioBlock(juce::AudioSourceChannelInfo(buffer));

    auto* gainParameter = parameters.getParameter("gain");
    float currentGain = gainParameter->getValue();

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        buffer.applyGain(channel, 0, buffer.getNumSamples(), currentGain);
    }
}

void AmbienceMachineAudioProcessor::loadFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);

    if (reader != nullptr)
    {
        std::unique_ptr<juce::AudioFormatReaderSource> newSource(new juce::AudioFormatReaderSource(reader, true));

        // Set the transport source to the new audio source
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);

        // Start playback
        transportSource.start();

        // Release the previous reader source and assign the new one
        readerSource.reset(newSource.release());
    }
}

void AmbienceMachineAudioProcessor::setGain(float gain)
{
    auto* gainParameter = parameters.getParameter("gain");
    gainParameter->setValueNotifyingHost(gain);
}

juce::AudioProcessorEditor* AmbienceMachineAudioProcessor::createEditor() { return new AmbienceMachineAudioProcessorEditor(*this); }
bool AmbienceMachineAudioProcessor::hasEditor() const { return true; }

//juce::AudioProcessor* createPluginFilter()
//{
//    return new AmbienceMachineAudioProcessor();
//}
juce::AudioProcessor* createPluginFilter()
{
    return new AmbienceMachineAudioProcessor();
}