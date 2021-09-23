#include "Logger.h"
#include "MicRecorder.h"

#pragma warning(push, 0)
#include <SpeechDetector.h>
#include <boost/program_options.hpp>
#include <cassert>
#include <queue>
#include <thread>
#include <iostream>
#include <memory>
#pragma warning(pop)

using namespace affdex;

namespace affdex_samples {

    /*
     * Connects to and retrieves audio from a microphone, processing it using the
     * Affectiva SpeechDetector.
     */
    class AudioHelper {

        /*
         * This sample demonstrates two possible ways of processing results
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

    public:
        std::unique_ptr<speech::SpeechDetector> detector;
        std::queue<std::unique_ptr<AudioChunk>> audioQueue;

        AudioHelper(path data_dir) : logger(new Logger) {
            detector.reset(new speech::SpeechDetector(data_dir));
            detector->enable(detector->getSupportedFeatures());

            // if using a listener, register the logger with the detector
#ifdef USE_LISTENER
            detector->setListener(logger.get());
#endif
            speech::Result r = detector->start();
            if (!r.isOK()) {
                std::cout << r.getErrorMsg();
                exit(1);
            }
        }

        /*
         * Listen to the microphone input for the specified time duration; passing the audio
         * received via a callback to the SpeechDetector for processing.
         */
        void processMicrophone(int recording_time_seconds) {

            MicRecorder mic(std::bind(&AudioHelper::micCallback, this, std::placeholders::_1));
            mic.start(recording_time_seconds);
            const speech::ProcessingResult result = detector->stop();

            // if not using a listener, process the return value
#ifndef USE_LISTENER
            logger->logResult(result);
#endif
        }

        void micCallback(AudioChunk chunk) {
            const speech::ProcessingResult result = detector->processAudio(chunk.timestamp_ms, chunk.samples.data(), chunk.samples.size());

            // if not using a listener, process the return value
#ifndef USE_LISTENER
            logger->logResult(result);
#endif
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
    int recording_time_seconds;

    namespace po = boost::program_options; // abbreviate namespace
    po::options_description description("Example project demonstrating the use of the Affectiva Speech library.");
    description.add_options()
        ("help,h", po::bool_switch()->default_value(false), "Display this help message.")
#ifdef _WIN32
        ("data,d", po::wvalue<path>(&data_dir)->required(), "the directory containing the model files")
#else //  _WIN32
        ("data,d", po::value<path>(&data_dir)->required(), "the directory containing the model files")
#endif // _WIN32
        ("recordingtime,t", po::value<int>(&recording_time_seconds)->default_value(500), "the time to record from the built-in microphone, in seconds (integer value)")
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
    std::cout << "Processing from microphone in real-time\n";
    h.processMicrophone(recording_time_seconds);

    return 0;
}
