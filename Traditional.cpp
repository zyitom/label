#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include "Traditional.h"
#include <fstream>
#include <iostream>
#include <QString>
TraditionalDetector::TraditionalDetector() {
    threshold = 0.5;
    model_path_ = "/home/zyi/Desktop/autoaim_ws/install/traditional_detectors/share/traditional_detectors/model/mlp.onnx";
    label_path_ = "/home/zyi/Desktop/autoaim_ws/install/traditional_detectors/share/traditional_detectors/model/label.txt";

    net = cv::dnn::readNetFromONNX(model_path_);
    std::ifstream label_file(label_path_);
    std::string line;
    while (std::getline(label_file, line)) {
        class_names.push_back(line);
    }
}

Armor TraditionalDetector::detectArmor(const cv::Mat& src, const Light& left_light, const Light& right_light) {
    Armor armor;
    armor.left_light = left_light;
    armor.right_light = right_light;

    armor.color = detectLightColor(src, left_light, right_light);
    armor.type = determineArmorType(armor);
    armor.number_img = extractNumber(src, armor);

    classifyNumber(armor);

    return armor;
}

void TraditionalDetector::classifyNumber(Armor& armor) {
    cv::Mat blob = cv::dnn::blobFromImage(armor.number_img, 1.0/255.0, cv::Size(20, 28), cv::Scalar(0, 0, 0), true, false);
    net.setInput(blob);
    cv::Mat outputs = net.forward();

    cv::Mat softmax_outputs;
    cv::exp(outputs, softmax_outputs);
    float sum = static_cast<float>(cv::sum(softmax_outputs)[0]);
    softmax_outputs /= sum;

    cv::Point classIdPoint;
    double confidence;
    cv::minMaxLoc(softmax_outputs.reshape(1, 1), nullptr, &confidence, nullptr, &classIdPoint);
    int label_id = classIdPoint.x;

    armor.number = class_names[label_id];
    armor.confidence = static_cast<float>(confidence);

    if (armor.confidence < threshold) {
        armor.number = "unknown";
    }
}

Armor::Color TraditionalDetector::detectLightColor(const cv::Mat& image, const Light& left_light, const Light& right_light) {
    cv::Scalar lower_red1(0, 100, 100), upper_red1(10, 255, 255);
    cv::Scalar lower_red2(160, 100, 100), upper_red2(180, 255, 255);
    cv::Scalar lower_blue(100, 100, 100), upper_blue(140, 255, 255);

    cv::Mat hsv_image;
    cv::cvtColor(image, hsv_image, cv::COLOR_BGR2HSV);

    cv::Mat mask = cv::Mat::zeros(image.size(), CV_8UC1);
    std::vector<cv::Point> light_points = {
        left_light.top, left_light.bottom,
        right_light.bottom, right_light.top
    };
    cv::fillConvexPoly(mask, light_points, cv::Scalar(255));

    cv::Mat masked_hsv;
    hsv_image.copyTo(masked_hsv, mask);

    cv::Mat red_mask1, red_mask2, blue_mask;
    cv::inRange(masked_hsv, lower_red1, upper_red1, red_mask1);
    cv::inRange(masked_hsv, lower_red2, upper_red2, red_mask2);
    cv::inRange(masked_hsv, lower_blue, upper_blue, blue_mask);

    int red_pixels = cv::countNonZero(red_mask1) + cv::countNonZero(red_mask2);
    int blue_pixels = cv::countNonZero(blue_mask);

    if (red_pixels > blue_pixels && red_pixels > 50) {
        return Armor::Color::RED;
    } else if (blue_pixels > red_pixels && blue_pixels > 50) {
        return Armor::Color::BLUE;
    } else {
        return Armor::Color::UNKNOWN;
    }
}

std::string TraditionalDetector::colorToString(Armor::Color color) {
    switch(color) {
        case Armor::Color::RED: return "RED";
        case Armor::Color::BLUE: return "BLUE";
        case Armor::Color::UNKNOWN: return "UNKNOWN";
        default: return "INVALID";
    }
}

cv::Mat TraditionalDetector::extractNumber(const cv::Mat& src, const Armor& armor) {
    const int light_length = 12;
    const int warp_height = 28;
    const int small_armor_width = 32;
    const int large_armor_width = 54;
    const cv::Size roi_size(20, 28);

    cv::Point2f lights_vertices[4] = {
        armor.left_light.bottom, armor.left_light.top,
        armor.right_light.top, armor.right_light.bottom
    };

    const int top_light_y = (warp_height - light_length) / 2 - 1;
    const int bottom_light_y = top_light_y + light_length;
    const int warp_width = armor.type == Armor::Type::SMALL ? small_armor_width : large_armor_width;

    cv::Point2f target_vertices[4] = {
        cv::Point(0, bottom_light_y),
        cv::Point(0, top_light_y),
        cv::Point(warp_width - 1, top_light_y),
        cv::Point(warp_width - 1, bottom_light_y),
    };

    cv::Mat number_image;
    auto rotation_matrix = cv::getPerspectiveTransform(lights_vertices, target_vertices);
    cv::warpPerspective(src, number_image, rotation_matrix, cv::Size(warp_width, warp_height));

    cv::Mat roi = number_image(cv::Rect(cv::Point((warp_width - roi_size.width) / 2, 0), roi_size));

    cv::Mat gray_image;
    cv::cvtColor(roi, gray_image, cv::COLOR_RGB2GRAY);

    cv::Mat binary_image;
    cv::threshold(gray_image, binary_image, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    cv::threshold(binary_image, binary_image, 128, 255, cv::THRESH_BINARY);

    return binary_image;
}

Armor::Type TraditionalDetector::determineArmorType(const Armor& armor) {
    const double min_light_ratio = 0.15;
    const double min_small_center_distance = 0.8;
    const double max_small_center_distance = 3.0;
    const double min_large_center_distance = 3.0;
    const double max_large_center_distance = 9.2;
    const double max_angle = 35.0;

    double left_light_height = cv::norm(armor.left_light.top - armor.left_light.bottom);
    double right_light_height = cv::norm(armor.right_light.top - armor.right_light.bottom);

    cv::Point2f left_center = (armor.left_light.top + armor.left_light.bottom) * 0.5;
    cv::Point2f right_center = (armor.right_light.top + armor.right_light.bottom) * 0.5;
    double center_distance = cv::norm(left_center - right_center);

    double avg_light_height = (left_light_height + right_light_height) * 0.5;

    double light_ratio = std::min(left_light_height, right_light_height) / std::max(left_light_height, right_light_height);
    if (light_ratio < min_light_ratio) {
        std::cout << "Invalid light ratio: " << light_ratio << std::endl;
        return Armor::Type::SMALL;
    }

    double relative_distance = center_distance / avg_light_height;

    if (relative_distance >= min_small_center_distance && relative_distance <= max_small_center_distance) {
        return Armor::Type::SMALL;
    } else if (relative_distance >= min_large_center_distance && relative_distance <= max_large_center_distance) {
        return Armor::Type::LARGE;
    } else {
        std::cout << "Invalid center distance: " << relative_distance << std::endl;
        return Armor::Type::SMALL;
    }

    double dx = right_center.x - left_center.x;
    double dy = right_center.y - left_center.y;
    double angle = std::abs(std::atan2(dy, dx) * 180 / CV_PI);
    if (angle > max_angle) {
        std::cout << "Invalid angle: " << angle << std::endl;
        return Armor::Type::SMALL;
    }

    return Armor::Type::SMALL;  // Default case
}

DetectionResult TraditionalDetector::detectAndClassify(const cv::Mat& src, const std::vector<cv::Point2f>& points) {
    if (points.size() < 4) {
        qDebug() << "Error: Not enough points provided for detection";
        return DetectionResult{0, 0, "unknown", 0.0f, Armor::Type::SMALL, Armor::Color::UNKNOWN};
    }

    Light left_light, right_light;
    left_light.top = points[0];
    left_light.bottom = points[1];
    right_light.top = points[3];
    right_light.bottom = points[2];

    Armor detectedArmor = detectArmor(src, left_light, right_light);

    DetectionResult result;
    result.color_id = detectedArmor.color == Armor::Color::RED ? 1 : 0;
    result.number = detectedArmor.number;
    result.confidence = detectedArmor.confidence;
    result.type = detectedArmor.type;
    result.color = detectedArmor.color;

    result.tag_id = assignTagId(detectedArmor);

    // 输出检测结果到控制台
    qDebug() << "Detected Armor:";
    qDebug() << "Number:" << QString::fromStdString(result.number);
    qDebug() << "Confidence:" << result.confidence;
    qDebug() << "Type:" << (result.type == Armor::Type::SMALL ? "SMALL" : "LARGE");
    qDebug() << "Color:" << QString::fromStdString(colorToString(result.color));
    qDebug() << "Assigned tag_id:" << result.tag_id;

    return result;
}

int TraditionalDetector::assignTagId(const Armor& armor) {
    int detectedNumber = -1;
    bool isNumber = false;

    if (!armor.number.empty()) {
        try {
            detectedNumber = std::stoi(armor.number);
            isNumber = true;
        } catch (const std::exception& e) {
            qDebug() << "Failed to convert number:" << QString::fromStdString(armor.number);
        }
    }

    if (armor.number == "outpost") {
        return 6;
    } else if (armor.number == "base") {
        return armor.type == Armor::Type::SMALL ? 7 : 8;
    } else if (armor.number == "guard") {
        return 0;
    } else if (isNumber) {
        if (detectedNumber == 1 || detectedNumber == 2) {
            return detectedNumber;
        } else if (detectedNumber >= 3 && detectedNumber <= 5) {
            if (armor.type == Armor::Type::SMALL) {
                return detectedNumber + 1; // 3, 4, 5 -> 4, 5, 6
            } else {
                return detectedNumber + 6; // 3, 4, 5 -> 9, 10, 11
            }
        }
    }

    qDebug() << "Warning: Unrecognized or invalid armor number:" << QString::fromStdString(armor.number);
    return 0; // 设置为默认值（guard）
}