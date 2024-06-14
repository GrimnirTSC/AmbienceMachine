#include "PluginProcessor.h"
#include "PluginEditor.h"

AmbienceMachineAudioProcessor::AmbienceMachineAudioProcessor()
    : parameters(*this, nullptr, "Parameters",
        {
            std::make_unique<juce::AudioParameterFloat>("gainAmbience", "Gain Ambience", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("gainRain", "Gain Rain", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("highpassRain", "Highpass Rain", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("gainOneshot", "Gain Oneshot", 0.0f, 1.0f, 0.5f)

        })
{
    formatManager.registerBasicFormats();
    gainParameterAmbience = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("gainAmbience"));
    gainParameterRain = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("gainRain"));
    gainParameterOneshot = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("gainOneshot"));

    highpassParameterRain = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("highpassRain"));

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

void AmbienceMachineAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ValueTree state = parameters.copyState();
    juce::MemoryOutputStream mos(destData, true);
    state.writeToStream(mos);
}

void AmbienceMachineAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ValueTree state = juce::ValueTree::readFromData(data, sizeInBytes);
    if (state.isValid())
    {
        parameters.replaceState(state);
    }
}

void AmbienceMachineAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    transportSourceAmbience.prepareToPlay(samplesPerBlock, sampleRate);
    transportSourceRain.prepareToPlay(samplesPerBlock, sampleRate);
    transportSourceOneshot.prepareToPlay(samplesPerBlock, sampleRate);


    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = getTotalNumInputChannels();
    highPassFilter.prepare(spec);

    float cutoffFrequency = 1000.0f;
    highPassFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, cutoffFrequency);
}

void AmbienceMachineAudioProcessor::releaseResources()
{
    transportSourceAmbience.releaseResources();
    transportSourceRain.releaseResources();
    transportSourceOneshot.releaseResources();
}

void AmbienceMachineAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Clear main output buffer
    buffer.clear();

    juce::AudioBuffer<float> tempBufferAmbience, tempBufferRain, tempBufferOneshot;
    tempBufferAmbience.setSize(buffer.getNumChannels(), buffer.getNumSamples(), false, true, true);
    tempBufferRain.setSize(buffer.getNumChannels(), buffer.getNumSamples(), false, true, true);
    tempBufferOneshot.setSize(buffer.getNumChannels(), buffer.getNumSamples(), false, true, true);

    // Process ambience if source is valid
    if (readerSourceAmbience.get() != nullptr)
    {
        juce::AudioSourceChannelInfo bufferToFillAmbience(tempBufferAmbience);
        transportSourceAmbience.getNextAudioBlock(bufferToFillAmbience);
        float gain = gainParameterAmbience->get();
        tempBufferAmbience.applyGain(gain);
    }

    // Process rain if source is valid
    if (readerSourceRain.get() != nullptr)
    {
        juce::AudioSourceChannelInfo bufferToFillRain(tempBufferRain);
        transportSourceRain.getNextAudioBlock(bufferToFillRain);
        float gain = gainParameterRain->get();
        tempBufferRain.applyGain(gain);

        // Apply high-pass filter to rain audio buffer
        juce::dsp::AudioBlock<float> rainBlock(tempBufferRain);
        juce::dsp::ProcessContextReplacing<float> context(rainBlock);
        highPassFilter.process(context);
    }

    if (readerSourceOneshot.get() != nullptr)
    {
        juce::AudioSourceChannelInfo bufferToFillOneshot(tempBufferOneshot);
        transportSourceOneshot.getNextAudioBlock(bufferToFillOneshot);
        float gain = gainParameterOneshot->get();
        tempBufferOneshot.applyGain(gain);

        // Check if oneshot playback has completed
        if (transportSourceOneshot.getNextReadPosition() >= transportSourceOneshot.getTotalLength())
        {
            // Start delay countdown
            if (!isWaitingForRestart)
            {
                isWaitingForRestart = true;
                restartTime = juce::Time::getMillisecondCounterHiRes() / 1000.0 + delayTimeInSeconds; // Calculate end time
            }

            // Check if delay time has passed
            if (isWaitingForRestart && juce::Time::getMillisecondCounterHiRes() / 1000.0 >= restartTime)
            {
                // Stop waiting and restart the transport
                isWaitingForRestart = false;
                transportSourceOneshot.setPosition(0);  // Set position to start
                transportSourceOneshot.start();
            }
        }
        else
        {
            // If oneshot is still playing, reset the delay state
            isWaitingForRestart = false;
        }
    }
    else
    {
        // If no oneshot is loaded, ensure transport is stopped
        transportSourceOneshot.stop();
        isWaitingForRestart = false; // Reset delay state
    }


    // Add processed buffers to main output buffer using addFrom
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        buffer.addFrom(channel, 0, tempBufferAmbience, channel, 0, tempBufferAmbience.getNumSamples());
        buffer.addFrom(channel, 0, tempBufferRain, channel, 0, tempBufferRain.getNumSamples());
        buffer.addFrom(channel, 0, tempBufferOneshot, channel, 0, tempBufferOneshot.getNumSamples());
    }

    // Check for clipping and panning issues
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float sampleValue = buffer.getSample(channel, sample);
            if (std::abs(sampleValue) > 1.0f)
            {
                juce::Logger::outputDebugString("Clipping detected at channel " + juce::String(channel) + ", sample " + juce::String(sample));
            }
        }
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

void AmbienceMachineAudioProcessor::loadOneshotFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);
    if (reader != nullptr)
    {
        std::unique_ptr<juce::AudioFormatReaderSource> newSource(new juce::AudioFormatReaderSource(reader, true));
        transportSourceOneshot.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        transportSourceOneshot.start();
        readerSourceOneshot.reset(newSource.release());
    }
}

void AmbienceMachineAudioProcessor::setGainAmbience(float gain)
{
    if (gainParameterAmbience != nullptr)
        gainParameterAmbience->setValueNotifyingHost(gain);
}

void AmbienceMachineAudioProcessor::setGainRain(float gain, float highpass)
{
    if (gainParameterRain != nullptr)
        gainParameterRain->setValueNotifyingHost(gain);
    if (highpassParameterRain != nullptr)
        highpassParameterRain->setValueNotifyingHost(highpass);

    // Calculate cutoff frequency inversely proportional to gain
    float invertedGain = 1.0f - gain;
    float cutoffFrequency = 20.0f + invertedGain * (10000.0f - 20.0f); // Example mapping: 20 Hz to 10000 Hz
    cutoffFrequency = juce::jlimit(20.0f, 10000.0f, cutoffFrequency); // Ensure within valid range
    highPassFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), cutoffFrequency);
}

void AmbienceMachineAudioProcessor::setGainOneshot(float gain)
{
    if (gainParameterOneshot != nullptr)
        gainParameterOneshot->setValueNotifyingHost(gain);
}

const int AmbienceMachineAudioProcessor::getParameterIDGainAmbience() const
{
    return gainParameterAmbience->getParameterIndex();
}

juce::AudioProcessorEditor* AmbienceMachineAudioProcessor::createEditor() { return new AmbienceMachineAudioProcessorEditor(*this); }
bool AmbienceMachineAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessor* createPluginFilter() { return new AmbienceMachineAudioProcessor(); }
