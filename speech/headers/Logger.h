#pragma once

#pragma warning(push, 0)
#include <SpeechDetector.h>
#include <iostream>
#include <iomanip>
#pragma warning(pop)


#define BEGIN_STREAM_PRECISION_CHANGE(X) const std::streamsize orig_precision = std::cout.precision(); const std::ios_base::fmtflags orig_flags = std::cout.flags(); std::cout << std::fixed << std::setprecision(X);
#define END_STREAM_PRECISION_CHANGE std::cout << std::setprecision(orig_precision); std::cout.flags(orig_flags);

namespace affdex_samples {
    class Logger : public affdex::speech::SpeechListener {
    public:
        void logResult(affdex::speech::ProcessingResult result) {
            if (result.hasStart())
                onSpeechStarted(result.speech_start_ms);
            if (result.hasSpeakerAnalysis())
                onSpeakerAnalyzed(result.speaker_analysis);
            if (result.hasSpeechAnalysis())
                onSpeechAnalyzed(result.speech_analysis);
            if (result.hasStop())
                onSpeechStopped(result.speech_stop_ms);
        }

        void onSpeechStarted(affdex::timestamp timestamp_ms) override {
            std::cout << "speech started: " << timestamp_ms << " ms\n";
        }

        void onSpeechStopped(affdex::timestamp timestamp_ms) override {
            std::cout << "speech stopped: " << timestamp_ms << " ms\n";
        }

        void onSpeakerAnalyzed(const affdex::speech::SpeakerAnalysis & analysis) override {
            std::cout << "---------------------------------------------\n";
            BEGIN_STREAM_PRECISION_CHANGE(4);
            std::cout << "Single Speaker: " << analysis.single_speaker << "\n";
            std::cout << "Multi Speaker: " << analysis.multi_speaker << "\n";
            std::cout << "Noise: " << analysis.noise << "\n";
            END_STREAM_PRECISION_CHANGE
            std::cout << "---------------------------------------------\n";
        }

        void onSpeechAnalyzed(const affdex::speech::SpeechAnalysis & analysis) override {

            std::string genderString = "I'm not sure of your gender based on your voice.";

            switch (analysis.gender_prediction) {
            case affdex::speech::Gender::MALE:
                genderString = "I believe you are MALE!\n";
                break;
            case affdex::speech::Gender::FEMALE:
                genderString = "I believe you are FEMALE!\n";
                break;
            case affdex::speech::Gender::UNKNOWN:
                genderString = "I'm not sure of your gender based on your voice.\n";
                break;
            }
            std::cout << "---------------------------------------------\n";
            std::cout << "results for time period [" << analysis.start_timestamp_ms << ", " << analysis.stop_timestamp_ms << ") ms\n";
            std::cout << "---------------------------------------------\n";
            std::cout << "Gender prediction: " << genderString;
            BEGIN_STREAM_PRECISION_CHANGE(4);
            std::cout << "Gender (male): " << analysis.gender_male << "\n";
            std::cout << "Gender (female): " << analysis.gender_female << "\n";
            std::cout << "Laughter: " << analysis.laughter << "\n";
            std::cout << "Arousal: " << analysis.arousal << "\n";
            std::cout << "Anger: " << analysis.anger << "\n";
            END_STREAM_PRECISION_CHANGE
        }
    };
}