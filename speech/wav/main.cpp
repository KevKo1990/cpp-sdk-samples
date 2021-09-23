#include "WAVReader.h"
#include "Logger.h"

#pragma warning(push, 0)
#include <boost/program_options.hpp>
#include <SpeechDetector.h>
#include <Exception.h>
#include <iostream>
#pragma warning(pop)

using namespace affdex;

namespace affdex_samples {

    /*
     * Reads audio samples from a file and processes them using the Affectiva SpeechDetector.
     */
    class AudioHelper {

    public:
        std::unique_ptr<speech::SpeechDetector> detector;

        /* This sample demonstrates two alternative ways of processing results
         * from the speech library.  One way is to define a listener class that
         * derives from affdex::speech::SpeechListener, and register it by
         * calling SpeechDetector::setListener().  The other way is to examine the
         * values returned from SpeechListener methods.  Note that since calls to
         * SpeechDetector methods will only return analysis results intermittently
         * (since audio input is accumulated and processed in batches), the
         * listener approach usually simpler, as it will be called when appropriate.
         * By defining or not defining USE_LISTENER, you can observe the different
         * approaches.
         */

        AudioHelper(path data_dir) : logger (new Logger()) {
            detector.reset(new speech::SpeechDetector(data_dir));
            detector->enable(detector->getSupportedFeatures());

            // if using a listener, register the logger with the detector
#ifdef USE_LISTENER
            detector->setListener(logger.get());
#endif
        }

        void processFile(path filename) {
            // setup to read audio from file
            WAVReader au = WAVReader();
            std::string wav(filename.begin(), filename.end());
            if (au.readWav(wav)) {

                try {
                    detector->start();
                } catch (Exception& e)
                {
                    std::cout << e.what();
                }
                speech::ProcessingResult result;

                const unsigned int samples_per_segment = au.sample_rate / 1000 * speech::SEGMENT_SIZE_MS_DEFAULT;

                // process the sample data; discarding leftover data at the end that isn't enough to fill a segment
                timestamp timestamp_ms = 0;
                for (unsigned long offset = 0; offset <= au.num_samples - samples_per_segment; offset += samples_per_segment) {
                    speech::audio_sample *buffer = &au.sound_data[offset];
                    result = detector->processAudio(timestamp_ms, buffer, samples_per_segment);

                    // if not using a listener, process the return value
#ifndef USE_LISTENER
                    logger->logResult(result);
#endif
                    timestamp_ms += speech::SEGMENT_SIZE_MS_DEFAULT;
                }

                result = detector->stop();

                // if not using a listener, process the return value
#ifndef USE_LISTENER
                logger->logResult(result);
#endif
            }
        }

        private:
            std::unique_ptr<Logger> logger;
    };

}

/*******************************************************************/
int main(int argc, char **argv)
{
    using namespace affdex_samples;

    path filename, data_dir;

    namespace po = boost::program_options; // abbreviate namespace
    po::options_description description("Example project demonstrating the use of the Affectiva Speech library.");
    description.add_options()
        ("help,h", po::bool_switch()->default_value(false), "Display this help message.")
#ifdef _WIN32
        ("file,f", po::wvalue<path>(&filename)->required(), "the filename to process (WAV file)")
        ("data,d", po::wvalue<path>(&data_dir)->required(), "the directory containing the model files")
#else //  _WIN32
        ("file,f", po::value<path>(&filename)->required(), "the filename to process (WAV file)")
        ("data,d", po::value<path>(&data_dir)->required(), "the directory containing the model files")
#endif // _WIN32
        ;
    po::variables_map args;
    try
    {
        store(po::command_line_parser(argc, argv).options(description).run(), args);
        if (args["help"].as<bool>())
        {
            std::cout << description << std::endl;
            return 0;

        }
        notify(args);
    }
    catch (po::error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << "For help, use the -h option." << std::endl << std::endl;
        return 1;
    }

    AudioHelper h(data_dir);
    std::cout << "Processing file: " << std::string(filename.begin(), filename.end()) << "\n";
    h.processFile(filename);

    return 0;
}
