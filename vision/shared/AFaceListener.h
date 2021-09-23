#pragma once

#include <FaceListener.h>
#include <iostream>

using namespace affdex;

class AFaceListener : public vision::FaceListener
{
    void onFaceFound(float timestamp, FaceId faceId)
    {
        std::cout << "Face id " << faceId << " found at timestamp " << timestamp << std::endl;
    }
    void onFaceLost(float timestamp, FaceId faceId)
    {
        std::cout << "Face id " << faceId << " lost at timestamp " << timestamp << std::endl;
    }
};
