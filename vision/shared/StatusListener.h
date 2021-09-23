#pragma once

#include <ProcessStatusListener.h>

#include <iostream>
#include <mutex>

using namespace affdex;

class StatusListener : public vision::ProcessStatusListener
{
public:

    StatusListener():is_running(true) {};

    void onProcessingException(Exception ex) override
    {
        std::cerr << "Encountered an exception while processing: " << ex.what() << std::endl;
        m.lock();
        is_running = false;
        m.unlock();
    };

    void onProcessingFinished() override
    {
        std::cerr << "Processing finished successfully" << std::endl;
        m.lock();
        is_running = false;
        m.unlock();

    };

    bool isRunning()
    {
        bool ret = true;
        m.lock();
        ret = is_running;
        m.unlock();
        return ret;
    };

private:
    std::mutex m;
    bool is_running;

};
