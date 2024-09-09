//
// Created by zyi on 24-9-9.
//
#ifndef NUMBER_RECOGNITION_H
#define NUMBER_RECOGNITION_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <QDebug>
#include <QString>
struct Light {
    cv::Point2f top;
    cv::Point2f bottom;
};

struct Armor {
    Light left_light;
    Light right_light;
    cv::Mat number_img;
    enum class Type { SMALL, LARGE } type;
    std::string number;
    float confidence;
    enum class Color { RED, BLUE, UNKNOWN } color;
};

struct DetectionResult {
    int tag_id;
    int color_id;
    std::string number;
    float confidence;
    Armor::Type type;
    Armor::Color color;
};

class TraditionalDetector {
public:
    TraditionalDetector();

    DetectionResult detectAndClassify(const cv::Mat& src, const std::vector<cv::Point2f>& points);

    static std::string colorToString(Armor::Color color);

    Armor detectArmor(const cv::Mat& src, const Light& left_light, const Light& right_light);
private:
    cv::dnn::Net net;
    std::vector<std::string> class_names;
    float threshold;
    std::string model_path_;
    std::string label_path_;


    void classifyNumber(Armor& armor);
    Armor::Color detectLightColor(const cv::Mat& image, const Light& left_light, const Light& right_light);
    cv::Mat extractNumber(const cv::Mat& src, const Armor& armor);
    Armor::Type determineArmorType(const Armor& armor);
    int assignTagId(const Armor& armor);
};

#endif //NUMBER_RECOGNITION_H