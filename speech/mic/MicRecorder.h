#pragma once

#pragma warning(push, 0)
#include <portaudio.h>
#include <SpeechDetector.h>
#include <queue>
#include <functional>
#include <memory>
#pragma warning(pop)



namespace affdex_samples {
    const int NUM_CHANNELS = 1;

    struct AudioChunk {
        AudioChunk(affdex::speech::audio_sample* samples, unsigned long sample_count, affdex::timestamp timestamp_ms) : timestamp_ms(timestamp_ms) {
            this->samples.insert(this->samples.begin(), samples, samples + sample_count);
        }

        std::vector<affdex::speech::audio_sample> samples;
        affdex::timestamp timestamp_ms;
    };

    class MicRecorder {
    public:

        MicRecorder(std::function<void(AudioChunk)> callback);
        void start(int recording_time_seconds);
        affdex::timestamp getTimestamp() const { return recording_timestamp_ms; }
        void push(std::unique_ptr<AudioChunk> audio_chunk);

    private:
        PaStream *stream;
        volatile bool is_running;
        std::function<void(AudioChunk)> callback;
        std::queue<std::unique_ptr<AudioChunk>> audio_queue;
        affdex::timestamp recording_timestamp_ms;

        void ConsumerThread();
    };
}
