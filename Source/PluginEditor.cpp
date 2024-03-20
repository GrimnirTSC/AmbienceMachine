/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AmbienceMachineAudioProcessorEditor::AmbienceMachineAudioProcessorEditor (AmbienceMachineAudioProcessor& p)
    : AudioProcessorEditor (&p), 
    audioProcessor (p), 
    PlayButton("Play"), 
    StopButton("Stop"), 
    OpenButton("Open")

    
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    addAndMakeVisible(&OpenButton);
}

AmbienceMachineAudioProcessorEditor::~AmbienceMachineAudioProcessorEditor()
{
}

//==============================================================================
void AmbienceMachineAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //g.setColour (juce::Colours::white);
    //g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
  /*  PlayButton.setColour();*/
}

void AmbienceMachineAudioProcessorEditor::resized()
{
    OpenButton.setBounds(10, 10, getWidth() - 20, 30);
}
