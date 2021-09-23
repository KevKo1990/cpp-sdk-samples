#include "MicRecorder.h"

#pragma warning(push, 0)
#include <thread>
#include <chrono>
#include <iostream>
#pragma warning(pop)

namespace affdex_samples {

    /* This routine will be called by the PortAudio engine when audio is ready.
     * Create AudioChunks from the received samples and enqueue them to be processed by a background thread.
     */
    static int recordCallback(const void *inputBuffer, void *, unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void *userData) {
        MicRecorder *mic_recorder = reinterpret_cast<MicRecorder*>(userData);

        // create an AudioChunk from the data provided by PortAudio, and add it to the processing queue
        affdex::speech::audio_sample* samples = reinterpret_cast<affdex::speech::audio_sample *>(const_cast<void*>(inputBuffer));
        mic_recorder->push(std::unique_ptr<AudioChunk>(new AudioChunk(samples, framesPerBuffer, mic_recorder->getTimestamp())));

        return 0;
    }

    MicRecorder::MicRecorder(std::function<void(AudioChunk)> callback) :
        stream(nullptr),
        is_running(false),
        callback(std::move(callback)),
        recording_timestamp_ms(0) {
        }

    /*
     * Capture audio from the microphone for a specified duration
     */
    void MicRecorder::start(int recording_time_seconds) {
        this->is_running = true;

        // background thread to process the AudioChunk queue
        std::thread consumer(&MicRecorder::ConsumerThread, this);

        PaStreamParameters  inputParameters;
        PaError err = paNoError;

        err = Pa_Initialize();
        if (err != paNoError) {
            std::cout << "failed to initialize PortAudio\n";
            return;
        }

        inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
        if (inputParameters.device == paNoDevice) {
            std::cout << "failed to identify default microphone\n";
            return;
        }

        inputParameters.channelCount = NUM_CHANNELS;
        inputParameters.sampleFormat = paInt16;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
        inputParameters.hostApiSpecificStreamInfo = nullptr;

        /* Record some audio. -------------------------------------------- */
        if (Pa_OpenStream(
                          &stream,
                          &inputParameters,
                          NULL,                  /* &outputParameters, */
                          affdex::speech::SAMPLE_RATE_DEFAULT,
                          affdex::speech::SAMPLES_PER_SEGMENT_DEFAULT,
                          paClipOff,      /* we won't output out of range samples so don't bother clipping them */
                          recordCallback,
                          this) != paNoError) {
            std::cout << "failed to open an audio stream on the microphone\n";
            return;
        }

        if (Pa_StartStream(stream) != paNoError) {
            std::cout << "failed to begin streaming audio from microphone\n";
            return;
        }

        // continue processing incoming audio for recording_time_seconds
        std::this_thread::sleep_for(std::chrono::seconds(recording_time_seconds));

        if (Pa_StopStream(stream) != paNoError)
        {
            std::cout << "failed to stop audio stream\n";
            return;
        }

        if (Pa_CloseStream(stream) != paNoError) {
            std::cout << "failed to close audio stream\n";
            return;
        }

        Pa_Terminate();

        is_running = false; // signal the consumer thread to finish up
        consumer.join();
    }

    /*
     * Push an AudioChunk onto the processing queue, and bump the timestamp
     */
    void MicRecorder::push(std::unique_ptr<AudioChunk> audio_chunk)
    {
        audio_queue.push(std::move(audio_chunk));
        recording_timestamp_ms += affdex::speech::SEGMENT_SIZE_MS_DEFAULT;
    }

    /*
     * As the recordCallback routine pushes AudioChunks onto audio_queue, this thread pops them off
     * and passes them to the registered callback.
     */
    void MicRecorder::ConsumerThread() {

        while (is_running) {
            // pop a chunk from the queue and pass it to the callback for processing
            if (!audio_queue.empty()) {
                const AudioChunk next_chunk = *audio_queue.front();
                this->callback(next_chunk);
                audio_queue.pop();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}
