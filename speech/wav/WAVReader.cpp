#include "WAVReader.h"

#pragma warning(push, 0)
#include <cassert>
#include <iostream>
#include <SpeechDetector.h>
#pragma warning(pop)

using namespace std;
using namespace affdex::speech;

namespace affdex_samples {
    bool WAVReader::readWav(const string& path) {
        bool result = false;
        SF_INFO sf_info;
        SNDFILE *sf = sf_open(path.c_str(), SFM_READ, &sf_info);
        if (sf) {
            sample_rate = sf_info.samplerate;
            unsigned int fo = sf_info.format;
            unsigned int ch = sf_info.channels;
            
            if (sample_rate != SAMPLE_RATE_DEFAULT) {
                cout << "unsupported sample rate: " << to_string(sample_rate) << endl;
                exit(-1);
            }

            if (fo != (SF_FORMAT_WAV | SF_FORMAT_PCM_16)) {
                cout << "unsupported file format\n";
                exit(-1);
            }

            if (ch != 1) {
                cout << "input audio file must be mono\n";
                exit(-1);
            }

            const unsigned int sample_width = 2; // assume 16 bit samples
            num_samples = sf_info.frames;

            // read ENTIRE file into one large buffer -- this is important for later post processing.
            // audio data MUST be contiguous.
            const sf_count_t total_bytes =  num_samples * sample_width;
            sound_data = new short[num_samples];
            const sf_count_t bytes_read = sf_read_raw(sf, sound_data, total_bytes);
            assert(bytes_read == total_bytes);

            result = true;
        } else {
            cout << "Error: not a RIFF file\n";
        }
        
        sf_close(sf);
        
        return result;
    }
}
