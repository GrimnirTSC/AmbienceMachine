#include "PluginProcessor.h"
#include "PluginEditor.h"

AmbienceMachineAudioProcessor::AmbienceMachineAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    analysis(),
    parameters(*this, nullptr, "Parameters",

        {
            std::make_unique<juce::AudioParameterFloat>("gainAmbience", "Gain Ambience", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("gainRain", "Gain Rain", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("highpassRain", "Highpass Rain", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("gainOneshot", "Gain Oneshot", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("FrequencyOneshot", "Frequency Oneshot", 0.0f, 40.0f, 0.5f)

        })
   
{
    formatManager.registerBasicFormats();
    gainParameterAmbience = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("gainAmbience"));
    gainParameterRain = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("gainRain"));
    gainParameterOneshot = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("gainOneshot"));

    highpassParameterRain = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("highpassRain"));

    FrequencyParameterOneshot = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("FrequencyOneshot"));


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
        float randomPan = juce::Random::getSystemRandom().nextFloat() * 5.0f - 1.0f; // Random pan between -10.0 and 10.0
        float pan = (randomPan + 10.0f) / 20.0f; // Normalize pan to range 0.0 to 1.0

        float leftGain = 1.0f - pan;
        float rightGain = pan;

        for (int channel = 0; channel < tempBufferOneshot.getNumChannels(); ++channel)
        {
            float channelGain = (channel == 0) ? leftGain : rightGain;

            // Apply gain to the channel
            tempBufferOneshot.applyGain(channel, 0, tempBufferOneshot.getNumSamples(), channelGain);
        }

        // Check if oneshot playback has completed
        if (transportSourceOneshot.getNextReadPosition() >= transportSourceOneshot.getTotalLength())
        {
            // Restart logic
            if (!isWaitingForRestart)
            {
                isWaitingForRestart = true;
                restartTime = juce::Time::getMillisecondCounterHiRes() / 1000.0 + FrequencyParameterOneshot->get(); // Calculate end time
            }

            // Check delay time
            if (isWaitingForRestart && juce::Time::getMillisecondCounterHiRes() / 1000.0 >= restartTime)
            {
                // Stop and restart transport
                isWaitingForRestart = false;
                transportSourceOneshot.setPosition(0);
                transportSourceOneshot.start();
            }
        }
        else
        {
            isWaitingForRestart = false;
        }
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
    juce::ScopedNoDenormals noDenormals;
    auto numInputChannels = getTotalNumInputChannels();
    auto numOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();

    // Clear any output channels that don't contain input data.
    for (auto i = numInputChannels; i < numOutputChannels; ++i) {
        buffer.clear(i, 0, numSamples);
    }

    bool stereo = numInputChannels > 1;
    const float* channelL = buffer.getReadPointer(0);
    const float* channelR = buffer.getReadPointer(stereo ? 1 : 0);

    float levelL = 0.0f;
    float levelR = 0.0f;
    float levelM = 0.0f;
    float levelS = 0.0f;

    for (int sample = 0; sample < numSamples; ++sample) {
        float sampleL = channelL[sample];
        float sampleR = channelR[sample];
        float sampleM = (sampleL + sampleR) * 0.5f;
        float sampleS = (sampleL - sampleR) * 0.5f;

        levelL = std::max(levelL, std::abs(sampleL));
        levelR = std::max(levelR, std::abs(sampleR));
        levelM = std::max(levelM, std::abs(sampleM));
        levelS = std::max(levelS, std::abs(sampleS));
    }

    analysis.levelL.update(levelL);
    analysis.levelR.update(levelR);
    analysis.levelM.update(levelM);
    analysis.levelS.update(levelS);
}

bool AmbienceMachineAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono();
}
void AmbienceMachineAudioProcessor::reset()
{
    analysis.reset();
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
void AmbienceMachineAudioProcessor::setFrequencyOneshot(float time)
{
    
    FrequencyParameterOneshot->setValueNotifyingHost(time);
}


const int AmbienceMachineAudioProcessor::getParameterIDGainAmbience() const
{
    return gainParameterAmbience->getParameterIndex();
}

juce::AudioProcessorEditor* AmbienceMachineAudioProcessor::createEditor() { return new AmbienceMachineAudioProcessorEditor(*this); }
bool AmbienceMachineAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessor* createPluginFilter() { return new AmbienceMachineAudioProcessor(); }
