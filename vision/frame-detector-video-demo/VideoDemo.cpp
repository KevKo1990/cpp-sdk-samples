#include "PlottingImageListener.h"
#include "StatusListener.h"

#include <FrameDetector.h>

#include <opencv2/highgui/highgui.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <iostream>

using namespace std;
using namespace affdex;

class VideoReader {
public:
    VideoReader(const boost::filesystem::path& video_file, const unsigned int sampling_frame_rate) :
        sampling_frame_rate(sampling_frame_rate) {

        if (!(sampling_frame_rate == -1 || sampling_frame_rate > 0))
            throw runtime_error("Specified sampling rate is <= 0");

        last_timestamp_ms = 0 - 1 / sampling_frame_rate; // Initialize it so we still get timestamp 0 with sampling

        cap.open(video_file.string());
        if (!cap.isOpened())
            throw runtime_error("Error opening video file: " + video_file.string());
    }

    bool GetFrame(cv::Mat &bgr_frame, timestamp& timestamp_ms) {
        bool frame_data_loaded;

        do {
            frame_data_loaded = GetFrameData(bgr_frame, timestamp_ms);
        } while ((sampling_frame_rate > 0)
            && (timestamp_ms > 0)
            && ((timestamp_ms - last_timestamp_ms) < 1000 / sampling_frame_rate)
            && frame_data_loaded);

        last_timestamp_ms = timestamp_ms;
        return frame_data_loaded;
    }

    bool GetFrameData(cv::Mat &bgr_frame, timestamp& timestamp_ms) {
        static const int MAX_ATTEMPTS = 2;
        timestamp prev_timestamp_ms = cap.get(::CV_CAP_PROP_POS_MSEC);
        bool frame_found = cap.grab();
        bool frame_retrieved = cap.retrieve(bgr_frame);
        timestamp_ms = cap.get(::CV_CAP_PROP_POS_MSEC);

        // Two conditions result in failure to decode (grab/retrieve) a video frame (timestamp reports 0):
        // (1) error on a particular frame
        // (2) end of the video file
        //
        // This workaround double-checks that a subsequent frame can't be decoded, in the absence
        // of better reporting on which case has been encountered.
        int n_attempts = 0;
        while (!(frame_found && frame_retrieved) && n_attempts++ < MAX_ATTEMPTS) {
            frame_found = cap.grab();
            frame_retrieved = cap.retrieve(bgr_frame);
            timestamp_ms = cap.get(::CV_CAP_PROP_POS_MSEC);
        }

        if (frame_found && frame_retrieved && n_attempts > 0) {
            if (timestamp_ms < prev_timestamp_ms) {
                frame_found = false;
            }
        }

        return frame_found && frame_retrieved;
    }

private:

    cv::VideoCapture cap;
    timestamp last_timestamp_ms;
    unsigned int sampling_frame_rate;
};


int main(int argsc, char ** argsv) {

    std::map<boost::filesystem::path, bool> SUPPORTED_VIDEO_EXTS  = { {boost::filesystem::path(".avi"), 1},
                                                                      {boost::filesystem::path(".mov"), 1},
                                                                      {boost::filesystem::path(".flv"), 1},
                                                                      {boost::filesystem::path(".webm"), 1},
                                                                      {boost::filesystem::path(".wmv"), 1},
                                                                      {boost::filesystem::path(".mp4"), 1} };


    const int precision = 2;
    std::cerr.precision(precision);
    std::cout.precision(precision);

    // cmd line args
    affdex::path data_dir;
    affdex::path video_path;
    unsigned int sampling_frame_rate;
    unsigned int processing_frame_rate = 0;
    bool draw_display;
    unsigned int num_faces;

    namespace po = boost::program_options; // abbreviate namespace

    po::options_description description("Project for demoing the Affectiva FrameDetector class (processing video files).");
    description.add_options()
    ("help,h", po::bool_switch()->default_value(false), "Display this help message.")
#ifdef _WIN32
    ("data,d", po::wvalue<affdex::path>(&data_dir)->default_value(affdex::path(L"data"), std::string("data")), "Path to the data folder")
    ("input,i", po::wvalue<affdex::path>(&video_path)->required(), "Video file to processs")
#else // _WIN32
    ("data,d", po::value< affdex::path >(&data_dir)->default_value(affdex::path("data"), std::string("data")), "Path to the data folder")
    ("input,i", po::value< affdex::path >(&video_path)->required(), "Video file to processs")
#endif // _WIN32
    ("sfps", po::value<unsigned int>(&sampling_frame_rate)->default_value(Detector::DEFAULT_PROCESSING_FRAMERATE), "Input sampling frame rate.")
    ("pfps", po::value<unsigned int>(&processing_frame_rate), "Max processing frame rate.")
    ("draw", po::value<bool>(&draw_display)->default_value(true), "Draw video on screen.")
    ("numFaces", po::value<unsigned int>(&num_faces)->default_value(1), "Number of faces to be tracked.")
    ;

    po::variables_map args;

    try {
        po::store(po::command_line_parser(argsc, argsv).options(description).run(), args);
        if (args["help"].as<bool>()) {
            std::cout << description << std::endl;
            return 0;
        }
        po::notify(args);
    }
    catch (po::error& e) {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << "For help, use the -h option." << std::endl << std::endl;
        return 1;
    }

    // if processing frame rate not specified, default to sampling frame rate
    if (processing_frame_rate ==0) {
        processing_frame_rate = sampling_frame_rate;
    }

    if (sampling_frame_rate > processing_frame_rate) {
        std::cerr << "Warning: sampling frame rate (" << sampling_frame_rate << ") should be <= processing frame rate ("  << processing_frame_rate << ") to avoid dropped frames\n";
    }

    const boost::filesystem::path video_ext = boost::filesystem::path(video_path).extension();
    if (!SUPPORTED_VIDEO_EXTS[video_ext]) {
        std::cerr << "Unsupported video file extension: " << video_ext << std::endl;
        return 1;
    }

    // check the data directory
    if (!boost::filesystem::exists(data_dir)) {
        std::cerr << "Data directory doesn't exist: " << std::string(data_dir.begin(), data_dir.end()) << std::endl;
        std::cerr << description << std::endl;
        return 1;
    }

    try {
        //initialize the output file
        boost::filesystem::path csv_path(video_path);
        csv_path.replace_extension(".csv");
        std::ofstream csv_file_stream(csv_path.c_str());

        if (!csv_file_stream.is_open()) {
            std::cerr << "Unable to open csv file " << csv_path << std::endl;
            return 1;
        }

        // the VideoReader will handle decoding frames from the input video file
        VideoReader video_reader(video_path, sampling_frame_rate);

        // if the sampling rate and the requested processing rate are the same, bump the processing rate
        // by one to overcome potential rounding issues when comparing inter-frame timestamp differences
        // against the processing rate
        if (sampling_frame_rate == processing_frame_rate)
            processing_frame_rate++;


        // create the FrameDetector
        vision::FrameDetector detector(data_dir, processing_frame_rate, num_faces);

        // prepare listeners
        PlottingImageListener image_listener(csv_file_stream, draw_display);
        StatusListener status_listener;

        if (!image_listener.validate(detector.getSupportedExpressions()) ||
            !image_listener.validate(detector.getSupportedEmotions()) ||
            !image_listener.validate(detector.getSupportedMeasurements())) {
            return 1;
        }

        // configure the FrameDetector by enabling features and assigning listeners
        detector.enable({ vision::Feature::EMOTIONS, vision::Feature::EXPRESSIONS });
        detector.setImageListener(&image_listener);
        detector.setProcessStatusListener(&status_listener);

        detector.start();

        cv::Mat mat;
        timestamp timestamp_ms;
        while (video_reader.GetFrame(mat, timestamp_ms)) {
            // create a Frame from the video input and process it with the FrameDetector
            vision::Frame f(mat.size().width, mat.size().height, mat.data, vision::Frame::ColorFormat::BGR, timestamp_ms);
            detector.process(f);

            // Since a FrameDetector processes frames asynchronously, and video decoding frame rates are typically
            // faster than FrameDetector processing frame rates, some intervention is needed if we want to avoid
            // sending frames to the FrameDetector much faster than it can process them, resulting in a lot of dropped
            // frames.  So, if the video sampling frame rate <= the processing frame rate, we infer
            // that the intention is to process all frames, which we can ensure by waiting for each frame to be processed
            // before sending to the next one.

            if (sampling_frame_rate <= processing_frame_rate) {
                image_listener.waitForResult();
            }

            image_listener.processResults();
         }

        detector.stop();
        csv_file_stream.close();

        std::cout << "Output written to file: " << csv_path << std::endl;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what();
        return 1;
    }

    return 0;
}
