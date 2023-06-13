#include "PluginProcessor.h"

// Polifonia del mio sintetizzatore (numero di voci)
#define NUM_VOICES 8

//==============================================================================
DemoSynthAudioProcessor::DemoSynthAudioProcessor()
     : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)), // Niente input
       parameters(*this, nullptr, "SynthSettings", { Parameters::createParameterLayout() })
{
    // Per prima cosa aggiungo uno o più SynthSound al synth
    mySynth.addSound(new MySynthSound());

    // Poi aggiungo tante voci di polifonia quante voglio che possa gestirne
    for (int v = 0; v < NUM_VOICES; ++v)
        mySynth.addVoice(new SimpleSynthVoice(Parameters::defaultAtk, Parameters::defaultDcy, Parameters::defaultSus, Parameters::defaultRel));

    // E poi attacco il processor ai parametri, come al solito
    Parameters::addListenerToAllParameters(parameters, this);
}

//==============================================================================

// Questo è l'unico metodo diverso rispetto a un normale plugin VST
bool DemoSynthAudioProcessor::acceptsMidi() const
{
    return true;
}

//==============================================================================
void DemoSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mySynth.setCurrentPlaybackSampleRate(sampleRate);

    for (int v = 0; v < mySynth.getNumVoices(); ++v)
        if (auto voice = dynamic_cast<SimpleSynthVoice*>(mySynth.getVoice(v)))
            voice->prepareToPlay(sampleRate, samplesPerBlock);
}

void DemoSynthAudioProcessor::releaseResources()
{
}

// Supporta mono o stereo out (input non presente, come definito nel costruttore)
bool DemoSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void DemoSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const auto numSamples = buffer.getNumSamples();

    // Pulisco il buffer (non c'è input, e le SynthVoice sommano, non sovrascrivono)
    buffer.clear();

    // Lascio che la classe Synthsiser faccia le sue magie
    mySynth.renderNextBlock(buffer, midiMessages, 0, numSamples);

    // Se l'output è stereo copio sul secondo canale il contenuto del primo
    if (getTotalNumOutputChannels() == 2)
        buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
}

//==============================================================================

void DemoSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DemoSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(ValueTree::fromXml(*xmlState));
}

void DemoSynthAudioProcessor::parameterChanged(const String& paramID, float newValue)
{
    // Il cambio di parametri va propagato a tutte le voci di polifonia del synth
    for (int v = 0; v < mySynth.getNumVoices(); ++v)
        if (auto voice = dynamic_cast<SimpleSynthVoice*>(mySynth.getVoice(v)))
        {
            if (paramID == Parameters::nameAtk)
                voice->setAttack(newValue);

            if (paramID == Parameters::nameDcy)
                voice->setDecay(newValue);

            if (paramID == Parameters::nameSus)
                voice->setSustain(newValue);

            if (paramID == Parameters::nameRel)
                voice->setRelease(newValue);
        }

}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DemoSynthAudioProcessor();
}
