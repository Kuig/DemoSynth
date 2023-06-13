#pragma once
#include <JuceHeader.h>

namespace Parameters
{
	static const String nameAtk = "A";
	static const String nameDcy = "D";
	static const String nameSus = "S";
	static const String nameRel = "R";

	static const float defaultAtk = 0.000f;
	static const float defaultDcy = 0.025f;
	static const float defaultSus = 0.600f;
	static const float defaultRel = 1.000f;

	static const float maxRmsTime = 1.0f;

	static AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
	{
		std::vector<std::unique_ptr<RangedAudioParameter>> params;

		params.push_back(std::make_unique<AudioParameterFloat>(nameAtk, "Attack (s)",    NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f), defaultAtk));
		params.push_back(std::make_unique<AudioParameterFloat>(nameDcy, "Decay (s)",     NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f), defaultDcy));
		params.push_back(std::make_unique<AudioParameterFloat>(nameSus, "Sustain (amp)", NormalisableRange<float>(0.0f, 1.00f, 0.010f, 0.5f), defaultSus));
		params.push_back(std::make_unique<AudioParameterFloat>(nameRel, "Release (s)",   NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f), defaultRel));

		return { params.begin(), params.end() };
	}

	static void addListenerToAllParameters(AudioProcessorValueTreeState& valueTreeState, AudioProcessorValueTreeState::Listener* listener)
	{
		std::unique_ptr<XmlElement> xml(valueTreeState.copyState().createXml());

		for (auto element : xml->getChildWithTagNameIterator("PARAM"))
		{
			const String& id = element->getStringAttribute("id");
			valueTreeState.addParameterListener(id, listener);
		}
	}
}