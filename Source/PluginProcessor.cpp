#include "PluginProcessor.h"
#include "PluginEditor.h"

AmbienceMachineAudioProcessor::AmbienceMachineAudioProcessor()
    : parameters(*this, nullptr, "Parameters",
        {
            std::make_unique<juce::AudioParameterFloat>("gainAmbience", "Gain Ambience", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("gainRain", "Gain Rain", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("higpassRain", "Highpass Rain", 0.0f, 1.0f, 0.5f)


        })
{
    formatManager.registerBasicFormats();
    gainParameterAmbience = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("gainAmbience"));
    gainParameterRain = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("gainRain"));
    highpassParameterRain = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("higpassRain"));
    //highPassChain.get<1>().coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), 1000.0f);

}

AmbienceMachineAudioProcessor::~AmbienceMachineAudioProcessor() {}

const juce::String AmbienceMachineAudioProcessor::getName() const { return JucePlugin_Name; }

bool AmbienceMachineAudioProcessor::acceptsMidi() const { return false; }
bool AmbienceMachineAudioProcessor::producesMidi() const { return false; }
bool AmbienceMachineAudioProcessor::isMidiEffect() const { return false; }
double AmbienceMachineAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int AmbienceMachineAudioProcessor::getNumPrograms() { return 0; }
int AmbienceMachineAudioProcessor::getCurrentProgram() { return 0; }
void AmbienceMachineAudioProcessor::setCurrentProgram(int index) {}
const juce::String AmbienceMachineAudioProcessor::getProgramName(int index) { return juce::String(); }
void AmbienceMachineAudioProcessor::changeProgramName(int index, const juce::String& newName) {}

//code responsible for storing parameter values 
void AmbienceMachineAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ValueTree state = parameters.copyState();

    // Convert the ValueTree object to binary data and store it in destData
    juce::MemoryOutputStream mos(destData, true);
    state.writeToStream(mos);
}
void AmbienceMachineAudioProcessor::setStateInformation(const void* data, int sizeInBytes) 
{
    // Convert the binary data in data to a ValueTree object
    juce::ValueTree state = juce::ValueTree::readFromData(data, sizeInBytes);

    // Check if the ValueTree is valid
    if (state.isValid())
    {
        // Apply the state to the parameters object
        parameters.replaceState(state);
    }

}


void AmbienceMachineAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    transportSourceAmbience.prepareToPlay(samplesPerBlock, sampleRate);
    transportSourceRain.prepareToPlay(samplesPerBlock, sampleRate);
 
}

void AmbienceMachineAudioProcessor::releaseResources()
{
    transportSourceAmbience.releaseResources();
    transportSourceRain.releaseResources();
}
void AmbienceMachineAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    juce::AudioBuffer<float> tempBufferAmbience, tempBufferRain;
    tempBufferAmbience.setSize(buffer.getNumChannels(), buffer.getNumSamples(), false, true, true);
    tempBufferRain.setSize(buffer.getNumChannels(), buffer.getNumSamples(), false, true, true);

    if (readerSourceAmbience.get() != nullptr)
    {
        juce::AudioSourceChannelInfo bufferToFillAmbience(tempBufferAmbience);
        transportSourceAmbience.getNextAudioBlock(bufferToFillAmbience);
        float gain = gainParameterAmbience->get(); // Get the current gain value
        tempBufferAmbience.applyGain(gain); // Apply the gain regardless of its value
    }

    if (readerSourceRain.get() != nullptr)
    {
        juce::AudioSourceChannelInfo bufferToFillRain(tempBufferRain);
        transportSourceRain.getNextAudioBlock(bufferToFillRain);
        tempBufferRain.applyGain(*gainParameterRain);

        // Apply high-pass filter to each channel of the rain audio buffer
        // for (int channel = 0; channel < tempBufferRain.getNumChannels(); ++channel)
        // {
        //     auto* channelData = tempBufferRain.getWritePointer(channel);
        //     highPassChain.processSamples(channelData, tempBufferRain.getNumSamples());
        // }
    }

    // Mix tempBufferAmbience and tempBufferRain into the main buffer
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        buffer.addFrom(channel, 0, tempBufferAmbience, channel, 0, tempBufferAmbience.getNumSamples());
        buffer.addFrom(channel, 0, tempBufferRain, channel, 0, tempBufferRain.getNumSamples());
    }
}


void AmbienceMachineAudioProcessor::loadAmbienceFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);

    if (reader != nullptr)
    {
        std::unique_ptr<juce::AudioFormatReaderSource> newSource(new juce::AudioFormatReaderSource(reader, true));
        transportSourceAmbience.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        transportSourceAmbience.start();
        readerSourceAmbience.reset(newSource.release());
    }
}

void AmbienceMachineAudioProcessor::loadRainFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);

    if (reader != nullptr)
    {
        std::unique_ptr<juce::AudioFormatReaderSource> newSource(new juce::AudioFormatReaderSource(reader, true));
        transportSourceRain.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        transportSourceRain.start();
        readerSourceRain.reset(newSource.release());
    }
}


void AmbienceMachineAudioProcessor::setGainAmbience(float gain)
{
    if (gainParameterAmbience != nullptr)
        gainParameterAmbience->setValueNotifyingHost(gain);

}

void AmbienceMachineAudioProcessor::setGainRain(float gain,float highpass)
{
    if (gainParameterRain != nullptr)
        gainParameterRain->setValueNotifyingHost(gain);
        highpassParameterRain->setValueNotifyingHost(highpass);
}

const int AmbienceMachineAudioProcessor::getParameterIDGainAmbience() const
{
    return gainParameterAmbience->getParameterIndex();
}

juce::AudioProcessorEditor* AmbienceMachineAudioProcessor::createEditor() { return new AmbienceMachineAudioProcessorEditor(*this); }
bool AmbienceMachineAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessor* createPluginFilter() { return new AmbienceMachineAudioProcessor(); }