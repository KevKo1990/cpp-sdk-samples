#include "AFaceListener.h"
#include "PlottingImageListener.h"
#include "StatusListener.h"

#include <FrameDetector.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <fstream>
#include <iostream>

using namespace std;
using namespace affdex;

int main(int argsc, char ** argsv) {
    namespace po = boost::program_options; // abbreviate namespace

    std::cerr << "Hit ESCAPE key to exit app.." << endl;

    try {

        const std::vector<int> DEFAULT_RESOLUTION {1280, 720};

        // cmd line options
        affdex::path data_dir;
        std::vector<int> resolution;
        int process_framerate;
        int camera_framerate;
        int camera_id;
        unsigned int num_faces;
        bool draw_display = true;

        const int precision = 2;
        std::cerr.precision(precision);
        std::cout.precision(precision);

        po::options_description description("Project for demoing the Affdex SDK FrameDetector class (grabbing and processing frames from the camera).");
        description.add_options()
            ("help,h", po::bool_switch()->default_value(false), "Display this help message.")
#ifdef _WIN32
            ("data,d", po::wvalue< affdex::path >(&data_dir)->default_value(affdex::path(L"data"), std::string("data")), "Path to the data folder")
#else //  _WIN32
            ("data,d", po::value< affdex::path >(&data_dir)->default_value(affdex::path("data"), std::string("data")), "Path to the data folder")
#endif // _WIN32
            ("resolution,r", po::value< std::vector<int> >(&resolution)->default_value(DEFAULT_RESOLUTION, "640 480")->multitoken(), "Resolution in pixels (2-values): width height")
            ("pfps", po::value< int >(&process_framerate)->default_value(30), "Processing framerate.")
            ("cfps", po::value< int >(&camera_framerate)->default_value(30), "Camera capture framerate.")
            ("cid", po::value< int >(&camera_id)->default_value(0), "Camera ID.")
            ("numFaces", po::value< unsigned int >(&num_faces)->default_value(1), "Number of faces to be tracked.")
            ("draw", po::value< bool >(&draw_display)->default_value(true), "Draw metrics on screen.")
            ;
        po::variables_map args;
        try {
            po::store(po::command_line_parser(argsc, argsv).options(description).run(), args);
            if (args["help"].as<bool>())
            {
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

        if (!boost::filesystem::exists(data_dir)) {
            std::cerr << "Data directory doesn't exist: " << std::string(data_dir.begin(), data_dir.end()) << std::endl << std::endl;;
            std::cerr << description << std::endl;
            return 1;
        }

        if (resolution.size() != 2) {
            std::cerr << "Only two numbers must be specified for resolution." << std::endl;
            return 1;
        }

        if (resolution[0] <= 0 || resolution[1] <= 0) {
            std::cerr << "Resolutions must be positive numbers." << std::endl;
            return 1;
        }

        // create the FrameDetector
        vision::FrameDetector frame_detector(data_dir, process_framerate, num_faces);

        // prepare listeners
        std::ofstream csvFileStream;
        PlottingImageListener image_listener(csvFileStream, draw_display);
        AFaceListener face_listener;
        StatusListener status_listener;

        if (!image_listener.validate(frame_detector.getSupportedExpressions()) ||
            !image_listener.validate(frame_detector.getSupportedEmotions()) ||
            !image_listener.validate(frame_detector.getSupportedMeasurements())) {
            return 1;
        }

        // configure the FrameDetector by enabling features and assigning listeners
        frame_detector.enable({ vision::Feature::EMOTIONS, vision::Feature::EXPRESSIONS });
        frame_detector.setImageListener(&image_listener);
        frame_detector.setFaceListener(&face_listener);
        frame_detector.setProcessStatusListener(&status_listener);

        // Connect to the webcam and configure it
        cv::VideoCapture webcam(camera_id);
        webcam.set(CV_CAP_PROP_FPS, camera_framerate);
        webcam.set(CV_CAP_PROP_FRAME_WIDTH, resolution[0]);
        webcam.set(CV_CAP_PROP_FRAME_HEIGHT, resolution[1]);

        const auto start_time = std::chrono::system_clock::now();
        if (!webcam.isOpened()) {
            std::cerr << "Error opening webcam" << std::endl;
            return 1;
        }

        //Start the frame detector thread.
        frame_detector.start();

        do {
            cv::Mat img;
            if (!webcam.read(img)) {   //Capture an image from the camera
                std::cerr << "Failed to read frame from webcam" << std::endl;
                break;
            }

            timestamp ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count();

            // Create a Frame from the webcam image and process it with the FrameDetector
            const vision::Frame f(img.size().width, img.size().height, img.data, vision::Frame::ColorFormat::BGR, ts);
            frame_detector.process(f);

            image_listener.processResults();
        }

#ifdef _WIN32
        while (!GetAsyncKeyState(VK_ESCAPE) && status_listener.isRunning());
#else //  _WIN32
        while (status_listener.isRunning());//(cv::waitKey(20) != -1);
#endif
        frame_detector.stop();
    }
    catch (std::exception& ex) {
        std::cerr << "Encountered an exception " << ex.what();
        return 1;
    }

    return 0;
}
