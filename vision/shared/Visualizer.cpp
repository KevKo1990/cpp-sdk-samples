#include "Visualizer.h"
#include "AffectivaLogo.h"

#include <opencv2/highgui/highgui.hpp>
#include <iomanip>

Visualizer::Visualizer():
  GREEN_COLOR_CLASSIFIERS({
    "joy"
  }),
  RED_COLOR_CLASSIFIERS({
    "anger"
  })
{
    logo_resized = false;
    logo = cv::imdecode(cv::InputArray(small_logo), CV_LOAD_IMAGE_UNCHANGED);

    EXPRESSIONS = {
        {affdex::vision::Expression::SMILE, "smile"},
        {affdex::vision::Expression::BROW_RAISE, "browRaise"},
        {affdex::vision::Expression::BROW_FURROW, "browFurrow"},
        {affdex::vision::Expression::NOSE_WRINKLE, "noseWrinkle"},
        {affdex::vision::Expression::UPPER_LIP_RAISE, "upperLipRaise"},
        {affdex::vision::Expression::MOUTH_OPEN, "mouthOpen"},
        {affdex::vision::Expression::EYE_CLOSURE, "eyeClosure"},
        {affdex::vision::Expression::CHEEK_RAISE, "cheekRaise"},
        {affdex::vision::Expression::YAWN, "yawn"},
        {affdex::vision::Expression::BLINK, "blink"},
        {affdex::vision::Expression::BLINK_RATE, "blinkRate"},
    };

    EMOTIONS = {
        {affdex::vision::Emotion::JOY, "joy"},
        {affdex::vision::Emotion::ANGER, "anger"},
        {affdex::vision::Emotion::SURPRISE, "surprise"},
        {affdex::vision::Emotion::VALENCE, "valence"},
    };

    HEAD_ANGLES = {
        {affdex::vision::Measurement::PITCH, "pitch"},
        {affdex::vision::Measurement::YAW, "yaw"},
        {affdex::vision::Measurement::ROLL, "roll"}
    };
}

void Visualizer::drawFaceMetrics(affdex::vision::Face face, std::vector<affdex::vision::Point> bounding_box)
{
    //Draw Right side metrics
    int padding = bounding_box[0].y; //Top left Y
    auto expressions = face.getExpressions();
    for (auto& exp : EXPRESSIONS) {
        // special case: display blink rate as number instead of bar
        if (exp.first == affdex::vision::Expression::BLINK_RATE) {
            std::stringstream ss;
            ss << std::fixed << std::setw(3) << std::setprecision(1);
            ss << expressions.at(exp.first);
            drawText(exp.second, ss.str(), cv::Point(bounding_box[1].x, padding += spacing), false, cv::Scalar(255,255,255));
        }
        else {
            float val = expressions.at(exp.first);
            if (exp.first == affdex::vision::Expression::BLINK) val *= 100; // blink is 0 or 1, so translate to 0 or 100 so it shows up in the UI
            drawClassifierOutput(exp.second, val, cv::Point(bounding_box[1].x, padding += spacing), false);
        }
    }

    padding = bounding_box[0].y;  //Top right Y
    //Draw Head Angles
    drawHeadOrientation(face.getMeasurements(), bounding_box[0].x - spacing, padding);

    //Draw Left side metrics
    auto emotions = face.getEmotions();
    for (auto& emo : EMOTIONS) {
        drawClassifierOutput(emo.second, emotions.at(emo.first), cv::Point(bounding_box[0].x, padding += spacing), true);
    }
}

void Visualizer::updateImage(cv::Mat output_img)
{
  img = output_img;

  if (!logo_resized)
  {
      double logo_width = (logo.size().width > img.size().width*0.25 ? img.size().width*0.25 : logo.size().width);
      double logo_height = ((double)logo_width) * ((double)logo.size().height / logo.size().width);
      cv::resize(logo, logo, cv::Size(logo_width, logo_height));
      logo_resized = true;
  }
  cv::Mat roi = img(cv::Rect(img.cols - logo.cols - 10, 10, logo.cols, logo.rows));
  overlayImage(logo, roi, cv::Point(0, 0));
}

void Visualizer::drawPoints(std::map<affdex::vision::FacePoint, affdex::vision::Point> points)
{
    for (auto& point : points)    //Draw face feature points.
    {
        cv::circle(img, cv::Point(point.second.x, point.second.y), 2.0f, cv::Scalar(255, 255, 255));
    }
}

void Visualizer::drawBoundingBox(std::vector<affdex::vision::Point> bounding_box, float valence)
{
    //Draw bounding box
    const ColorgenRedGreen valence_color_generator( -100, 100 );
    cv::Point top_left(bounding_box[0].x, bounding_box[0].y);
    cv::Point bottom_right(bounding_box[1].x, bounding_box[1].y);
    cv::rectangle( img, top_left, bottom_right,
                   valence_color_generator(valence), 3);

}

/** @brief DrawText prints text on screen either right or left justified at the anchor location (loc)
 * @param output_img  -- Image we are plotting on
 * @param name        -- Name of the classifier
 * @param value       -- Value we are trying to display
 * @param loc         -- Exact location. When aligh_right is (true/false) this should be the (upper-right, upper-left)
 * @param align_right -- Whether to right or left justify the text
 * @param color         -- Color
 */
void Visualizer::drawText(const std::string& name, const std::string& value,
                          const cv::Point2f loc, bool align_right, cv::Scalar color)
{
    const int block_width = 8;
    const int margin = 2;
    const int block_size = 10;
    const int max_blocks = 100/block_size;

    cv::Point2f display_loc = loc;
    const std::string label = name+": ";

    if( align_right )
    {
        display_loc.x -= (margin+block_width) * max_blocks;
        int baseline=0;
        cv::Size txtSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5f, 5,&baseline);
        display_loc.x -= txtSize.width;
    }
    cv::putText(img, label+value, display_loc, cv::FONT_HERSHEY_SIMPLEX, 0.5f, color, 1);
}



/** @brief DrawClassifierOutput handles choosing between equalizer or text as well as defining the colors
 * @param name        -- Name of the classifier
 * @param value       -- Value we are trying to display
 * @param loc         -- Exact location. When aligh_right is (true/false) this should be the (upper-right, upper-left)
 * @param align_right -- Whether to right or left justify the text
 */
void Visualizer::drawClassifierOutput(const std::string& classifier,
                                      const float value, const cv::Point2f& loc, bool align_right)
{

    static const ColorgenLinear white_yellow_generator( 0, 100, cv::Scalar(255,255,255), cv::Scalar(0, 255, 255));
    static const ColorgenRedGreen valence_color_generator( -100, 100 );

    // Determine the display color
    cv::Scalar color = cv::Scalar(255, 255, 255);
    if( classifier == "valence")
    {
        color = valence_color_generator( value );
    }
    else if( RED_COLOR_CLASSIFIERS.count(classifier) )
    {
        color = cv::Scalar(0, 0, 255);
    }
    else if( GREEN_COLOR_CLASSIFIERS.count(classifier) )
    {
        color = cv::Scalar(0, 255, 0);
    }

    float equalizer_magnitude = value;
    if( classifier == "valence" )
    {
        equalizer_magnitude = std::fabs(value);
    }
    drawEqualizer(classifier, equalizer_magnitude, loc, align_right, color );
}

void Visualizer::drawEqualizer(const std::string& name, const float value, const cv::Point2f& loc,
                               bool align_right, cv::Scalar color)
{
    const int block_width = 8;
    const int block_height = 10;
    const int margin = 2;
    const int block_size = 10;
    const int max_blocks = 100/block_size;
    int blocks = round(value / block_size);
    int i = loc.x, j = loc.y - 10;

    cv::Point2f display_loc = loc;
    const std::string label = align_right? name+": " : " :"+name;

    for (int x = 0 ; x < (100/block_size) ; x++)
    {
        cv::Scalar scalar_clr = color;
        float  alpha = 0.8;
        const int ii = (std::max)( float(i), 0.0f);
        const int jj = (std::max)( float(j), 0.0f);
        const int width = (std::min)(float(block_width), float(img.size().width-ii));
        const int height = (std::min)(float(block_height), float(img.size().height-jj));
        if (height < 0 || width < 0) continue;
        cv::Mat roi = img(cv::Rect(ii, jj, width, height));
        if (x >= blocks)
        {
            alpha = 0.3;
            scalar_clr = cv::Scalar(186, 186, 186);
        }
        cv::Mat color(roi.size(), CV_8UC3, scalar_clr);
        cv::addWeighted(color, alpha, roi, 1.0 - alpha , 0.0, roi);

        i += align_right? -(margin+block_width):(margin+block_width);
    }
    display_loc.x += align_right? -(margin+block_width) * max_blocks : (margin+block_width) * max_blocks;
    if( align_right )
    {
        int baseline=0;
        cv::Size txtSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5f, 5,&baseline);
        display_loc.x -= txtSize.width;
    }
    cv::putText(img, label, display_loc, cv::FONT_HERSHEY_SIMPLEX, 0.5f, cv::Scalar(50,50,50), 5);
    cv::putText(img, label, display_loc, cv::FONT_HERSHEY_SIMPLEX, 0.5f, cv::Scalar(255, 255, 255), 1);

}

void Visualizer::drawHeadOrientation(std::map<affdex::vision::Measurement, float> headAngles, const int x, int &padding,
                                     bool align_right, cv::Scalar color)
{
    std::stringstream ss;
    ss << std::fixed << std::setw(3) << std::setprecision(1);
    for (auto& h: HEAD_ANGLES) {
        ss << headAngles.at(h.first);
        drawText(h.second, ss.str(), cv::Point(x, padding += spacing), align_right, color );
        ss.str(""); // clear the string.
    }
}

void Visualizer::showImage()
{
    cv::imshow("analyze video", img);
    cv::waitKey(5);
}

void Visualizer::overlayImage(const cv::Mat &foreground, cv::Mat &background, cv::Point2i location)
{

    // start at the row indicated by location, or at row 0 if location.y is negative.
    for(int y = (std::max)(location.y , 0); y < background.rows; ++y)
    {
        int fY = y - location.y; // because of the translation

        // we are done of we have processed all rows of the foreground image.
        if(fY >= foreground.rows)
            break;

        // start at the column indicated by location,

        // or at column 0 if location.x is negative.
        for(int x = (std::max)(location.x, 0); x < background.cols; ++x)
        {
            int fX = x - location.x; // because of the translation.

            // we are done with this row if the column is outside of the foreground image.
            if(fX >= foreground.cols)
                break;

            // determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
            double opacity =
                    ((double)foreground.data[fY * foreground.step + fX * foreground.channels() + (foreground.channels()-1)])

                    / 255.;


            // and now combine the background and foreground pixel, using the opacity,

            // but only if opacity > 0.
            for(int c = 0; opacity > 0 && c < background.channels(); ++c)
            {
                unsigned char foregroundPx =
                        foreground.data[fY * foreground.step + fX * foreground.channels() + c];
                unsigned char backgroundPx =
                        background.data[y * background.step + x * background.channels() + c];
                background.data[y*background.step + background.channels()*x + c] =
                        backgroundPx * (1.-opacity) + foregroundPx * opacity;
            }
        }
    }
}

cv::Scalar ColorgenRedGreen::operator()( const float val ) const
{
    float norm_val = ( val - red_val_ ) / ( green_val_ - red_val_ );
    norm_val = norm_val < 0.0 ? 0.0 : norm_val;
    norm_val = norm_val > 1.0 ? 1.0 : norm_val;
    const int B = 0;
    const int G = norm_val * 255;
    const int R = ( 1.0 - norm_val ) * 255;
    return cv::Scalar( B, G, R );
}


cv::Scalar ColorgenLinear::operator()( const float val ) const
{
    float norm_val = ( val - val1_ ) / ( val2_ - val1_ );
    const int B = color1_.val[0] * (1.0f-norm_val) + color2_.val[0]*norm_val;
    const int G = color1_.val[1] * (1.0f-norm_val) + color2_.val[1]*norm_val;
    const int R = color1_.val[2] * (1.0f-norm_val) + color2_.val[2]*norm_val;
    return cv::Scalar( B, G, R );
}
