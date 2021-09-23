#pragma once

#pragma warning(push, 0)
#include <sndfile.h>
#include <string>
#pragma warning(pop)

namespace affdex_samples {
    class WAVReader {
    public:
        bool readWav(const std::string& path);

        short *sound_data;
        unsigned int sample_rate;
        sf_count_t num_samples;
    };
}
