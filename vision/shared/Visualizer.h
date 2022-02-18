#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include <Face.h>
#include <set>

/** @brief Plot the face metrics using opencv highgui
 */
class Visualizer
{
public:

    Visualizer();

    /** @brief UpdateImage refreshes the image that will be update
    * @param output_img  -- The image to display output on
    */
    void updateImage(cv::Mat output_img);

    /** @brief DrawPoints displays the landmark points on the image
    * @param points  -- The landmark points
    */
    void drawPoints(std::map<affdex::vision::FacePoint, affdex::vision::Point> points);

    /** @brief DrawBoundingBox displays the bounding box
    * @param top_left      -- The top left point
    * @param bottom_right  -- The bottom right point
    * @param valence       -- The valence value
    */
    void drawBoundingBox(std::vector<affdex::vision::Point>bounding_box, float valence);

    /** @brief DrawHeadOrientation Displays head orientation and associated value
    * @param name        -- Name of the classifier
    * @param value       -- Value we are trying to display
    * @param x           -- The x value of the location
    * @param padding     -- The padding value
    * @param align_right -- Whether to right or left justify the text
    * @param color       -- Color
    */
    void drawHeadOrientation(std::map<affdex::vision::Measurement, float> headAngles, const int x, int &padding,
                             bool align_right=true, cv::Scalar color=cv::Scalar(255,255,255));


    /** @brief DrawFaceMetrics Displays all facial metrics and associated value
    * @param face         -- The affdex::Face object to display
    * @param bounding_box -- The bounding box coordinates
    */
    void drawFaceMetrics(affdex::vision::Face face, std::vector<affdex::vision::Point> bounding_box);

    /** @brief ShowImage displays image on screen
    */
    void showImage();

    std::vector<std::pair<affdex::vision::Expression, std::string>> EXPRESSIONS;
    std::vector<std::pair<affdex::vision::Emotion, std::string>> EMOTIONS;
    std::vector<std::pair<affdex::vision::Measurement, std::string>> HEAD_ANGLES;

private:

    /**
    * Overlay an image with an Alpha (foreground) channel over background
    * Assumes foreground.size() == background.size()
    * Adapted from : http://jepsonsblog.blogspot.com/2012/10/overlay-transparent-image-in-opencv.html
    * @param foreground - image to overlay
    * @param background - ROI to overlay on
    */
    void overlayImage(const cv::Mat &foreground, cv::Mat &background, cv::Point2i location);

    /** @brief DrawClassifierOutput Displays a classifier and associated value
    * @param name        -- Name of the classifier
    * @param value       -- Value we are trying to display
    * @param loc         -- Exact location. When aligh_right is (true/false) this should be the (upper-right, upper-left)
    * @param align_right -- Whether to right or left justify the text
    */
    void drawClassifierOutput(const std::string& classifier, const float value,
                              const cv::Point2f& loc, bool align_right=false );

    /** @brief DrawEqualizer displays an equalizer on screen either right or left justified at the anchor location (loc)
    * @param name        -- Name of the classifier
    * @param value       -- Value we are trying to display
    * @param loc         -- Exact location. When aligh_right is (true/false) this should be the (upper-right, upper-left)
    * @param align_right -- Whether to right or left justify the text
    * @param color       -- Color
    */
    void drawEqualizer(const std::string& name, const float value, const cv::Point2f& loc,
                       bool align_right, cv::Scalar color);

    /** @brief DrawText displays an text on screen either right or left justified at the anchor location (loc)
    * @param name        -- Name of the classifier
    * @param value       -- Value we are trying to display
    * @param loc         -- Exact location. When aligh_right is (true/false) this should be the (upper-right, upper-left)
    * @param align_right -- Whether to right or left justify the text
    * @param color       -- Color
    */
    void drawText(const std::string& name, const std::string& value,
                  const cv::Point2f loc, bool align_right=false, cv::Scalar color=cv::Scalar(255,255,255));

    std::set<std::string> GREEN_COLOR_CLASSIFIERS;
    std::set<std::string> RED_COLOR_CLASSIFIERS;

    cv::Mat img;
    cv::Mat logo;
    bool logo_resized;
    const int spacing = 20;
    const int LOGO_PADDING = 20;
};

/** @brief Color generator (linear) for red-to-green values
 */
class ColorgenRedGreen
{
public:
    /** @brief ColorgenRedGreen
     * @param[in] red_val - Value which will return green
     * @param green_val - Value which will return green
     */
    ColorgenRedGreen( const float red_val, const float green_val )
        : red_val_(red_val),
          green_val_(green_val)
    {}

    /** @brief Generate accessor
     * @param val -- Value for which we would like to generate a color
     * @return  BGR Scalar for use in open cv plotting functions (e.g. cv::circle)
     */
    cv::Scalar operator()( const float val ) const;

private:
    const float red_val_;
    const float green_val_;
};

/**
 * @brief Color generator (linear) between any two colors
 */
class ColorgenLinear
{
public:
    ColorgenLinear( const float val1, const float val2, cv::Scalar color1, cv::Scalar color2 )
        :
          val1_(val1),
          val2_(val2),
          color1_(color1),
          color2_(color2)
    {}

    /** @brief Generate accessor
     * @param val -- Value for which we would like to generate a color
     * @return  BGR Scalar for use in open cv plotting functions (e.g. cv::circle)
     */
    cv::Scalar operator()( const float val ) const;

private:
  const float val1_;
  const float val2_;

  const cv::Scalar color1_;
  const cv::Scalar color2_;

};
