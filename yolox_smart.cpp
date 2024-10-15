#include "drawonpic.h"
#include <iostream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
BaseNetDetectorParams YoloxSmartModel::loadConfig(const std::string& config_path, int detect_mode) {
    YAML::Node config = YAML::LoadFile(config_path);
    std::cout << "Loaded config from " << config_path << std::endl;
    BaseNetDetectorParams params;

    try {
        params.classifier_threshold = config["detector"]["classifier_threshold"].as<double>();

        if (detect_mode == 0) {
            auto armor_paras = config["detector"]["armor_paras"];
            params.net_params.MODEL_PATH = armor_paras["model_path"].as<std::string>();
            params.net_params.NUM_CLASS = armor_paras["num_class"].as<int>();
            params.net_params.NUM_COLORS = armor_paras["num_colors"].as<int>();
            params.net_params.NMS_THRESH = armor_paras["nms_thresh"].as<float>();
            params.net_params.NUM_APEX = armor_paras["num_apex"].as<int>();
            INPUT_W = armor_paras["input_w"].as<int>();
            INPUT_H = armor_paras["input_h"].as<int>();
            std::cout<<INPUT_H<<std::endl;
            std::cout<<INPUT_W<<std::endl;
        } else {
            auto energy_paras = config["detector"]["energy_paras"];
            params.net_params.MODEL_PATH = energy_paras["model_path"].as<std::string>();
            params.net_params.NUM_CLASS = energy_paras["num_class"].as<int>();
            params.net_params.NUM_COLORS = energy_paras["num_colors"].as<int>();
            params.net_params.NMS_THRESH = energy_paras["nms_thresh"].as<float>();
            params.net_params.NUM_APEX = energy_paras["num_apex"].as<int>();
            INPUT_W = energy_paras["input_w"].as<int>();
            INPUT_H = energy_paras["input_h"].as<int>();
        }
    } catch (const YAML::TypedBadConversion<int>& e) {
        std::cerr << "Error converting to int: " << e.what() << std::endl;
        // 可以在这里设置默认值或重新抛出异常
        throw;
    } catch (const YAML::Exception& e) {
        std::cerr << "Error parsing YAML: " << e.what() << std::endl;
        throw;
    }

    return params;
}
YoloxSmartModel::YoloxSmartModel(int detect_mode = 0) {
    detect_mode_ = detect_mode;
            std::cout<<"12343432qweqw4"<<std::endl;
    std::string config_path = "/home/zyi/label/config.yaml";
try {
    params_ = loadConfig(config_path, detect_mode_);
            std::cout<<"12343432qweqw4"<<std::endl;
    
} catch (const std::exception& e) {
    std::cerr << "Error loading config: " << e.what() << std::endl;
}
    net_ = cv::dnn::readNet(params_.net_params.MODEL_PATH);
    net_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU); 
    std::cout << "YoloxSmartModel initialized with:" << std::endl;
    std::cout << "  NUM_CLASS: " << params_.net_params.NUM_CLASS << std::endl;
    std::cout << "  NUM_COLORS: " << params_.net_params.NUM_COLORS << std::endl;
    std::cout << "  NUM_APEX: " << params_.net_params.NUM_APEX << std::endl;
    std::cout << "Network input size: " << INPUT_W << "x" << INPUT_H << std::endl;
    std::cout << "Classifier threshold: " << params_.classifier_threshold << std::endl;
}

bool YoloxSmartModel::run(const QString &image_file, QVector<box_t> &boxes) {
    try {
    cv::Mat pre_img = static_resize(cv::imread(image_file.toStdString()));


    
    // Create a 4D blob from a frame
    cv::Mat blob;
    cv::dnn::blobFromImage(pre_img, blob, 1.0, cv::Size(INPUT_W, INPUT_H), cv::Scalar(), true, false);

    
    // Run a model
       net_.setInput(blob);
    std::vector<cv::Mat> outputs;
    net_.forward(outputs, net_.getUnconnectedOutLayersNames());
  
    // Post-processing
       std::vector<Object> objects;
    decode(outputs[0].ptr<float>(), objects, scale_);

   
        //显示推理图片
        cv::Mat show_img = pre_img.clone();
        for (auto &obj : objects) {
            cv::rectangle(show_img, obj.rect, cv::Scalar(0, 255, 0), 2);
            for (int i = 0; i < params_.net_params.NUM_APEX; i++) {
                cv::circle(show_img, obj.apexes[i], 2, cv::Scalar(0, 0, 255), 2);
            }
        }
        cv::imshow("YOLOX", show_img);
        cv::waitKey(0);
     
        boxes.clear();
        
      
        for (auto &obj : objects) {
            box_t box;
            
          
            box.conf = obj.conf;
            
           
            for (int i = 0; i < 4; i++) {
                box.pts[i].rx() = obj.apexes[i].x;
                box.pts[i].ry() = obj.apexes[i].y;
            }
            
         
            if (detect_mode_ == 0) {  
                box.color_id = obj.color;
                box.tag_id = obj.label;
            } else { // 能量机关模式
                box.color_id = obj.color;
                box.tag_id = obj.label;

            }
            
            // 将处理后的 box 添加到 boxes 向量中
            boxes.append(box);
        }
        
        return true; 
    } catch (const std::exception &e) {
        std::cerr << "Error in YoloxSmartModel::run: " << e.what() << std::endl;
        return false;  
    }
}








/**
 * @brief 获取某一段内最大值所在位置
*/
int YoloxSmartModel::argmax(const float* ptr, int len) {
    int arg_max = 0;
    for (int i = 1; i < len; i++) {
        if (ptr[i] > ptr[arg_max]) {
            arg_max = i;
        }
    }
    return arg_max;
}
cv::Mat YoloxSmartModel::static_resize(cv::Mat src) {
    if (src.empty()) {
        throw std::runtime_error("Input image is empty");
    }

    if (INPUT_W <= 0 || INPUT_H <= 0) {
        throw std::runtime_error("Invalid INPUT_W or INPUT_H");
    }

    scale_x_ = INPUT_W / static_cast<double>(src.cols);
    scale_y_ = INPUT_H / static_cast<double>(src.rows);

    if (scale_x_ <= 0 || scale_y_ <= 0) {
        throw std::runtime_error("Invalid scale computed: scale_x = " + 
                                 std::to_string(scale_x_) + 
                                 ", scale_y = " + 
                                 std::to_string(scale_y_));
    }

    cv::Mat out;
    try {
        cv::resize(src, out, cv::Size(INPUT_W, INPUT_H));
    } catch (const cv::Exception& e) {
        throw std::runtime_error("OpenCV resize failed: " + std::string(e.what()));
    }

    return out;
}

void YoloxSmartModel::generate_grids_and_stride(const int w, const int h, const int strides[], std::vector<GridAndStride> &grid_strides) {
    for(int i=0; i<3; i++){
        int num_grid_w = w/strides[i];
        int num_grid_h = h/strides[i];
        std::cout<<num_grid_w<<std::endl;
        for(int g1 = 0; g1<num_grid_h; g1++){
            for(int g0=0; g0<num_grid_w; g0++){
                grid_strides.emplace_back((GridAndStride{g0, g1, strides[i]}));
            }
        }
    }
}

void YoloxSmartModel::generate_yolox_proposal(std::vector<GridAndStride> &grid_strides, const float * output_buffer, float prob_threshold, std::vector<Object>& objects, float scale) {

    const int num_anchors = grid_strides.size();
    const int class_start = 2 * params_.net_params.NUM_APEX + 1;

    for(int anchor_idx = 0; anchor_idx<num_anchors; anchor_idx++){
        const int basic_pos = anchor_idx * (class_start +
            params_.net_params.NUM_CLASS +
            params_.net_params.NUM_COLORS);
        float box_conf = output_buffer[basic_pos+2 * params_.net_params.NUM_APEX];
        if (box_conf < prob_threshold) {
            continue;
        }

        // RCLCPP_INFO(rclcpp::get_logger("energy"), "bbbbb: %f", box_conf);

        const int grid0 = grid_strides[anchor_idx].grid0;
        const int grid1 = grid_strides[anchor_idx].grid1;
        const int stride = grid_strides[anchor_idx].stride;
        //4个点的xy坐标+1个置信度+颜色+类别

        std::vector<cv::Point2f> point;

        for (int i = 0; i < params_.net_params.NUM_APEX; i++) {
            point.emplace_back(cv::Point2f{((output_buffer[basic_pos + 0 + i * 2] + grid0) * stride) / scale_x_,
                                            ((output_buffer[basic_pos + 1 + i * 2] + grid1) * stride) / scale_y_
            });
        }
        //4个关键点
        // float x1 = (output_buffer[basic_pos+0]+grid0)*stride/scale;
        // float y1 = (output_buffer[basic_pos+1]+grid1)*stride/scale;
        // float x2 = (output_buffer[basic_pos+2]+grid0)*stride/scale;
        // float y2 = (output_buffer[basic_pos+3]+grid1)*stride/scale;
        // float x3 = (output_buffer[basic_pos+4]+grid0)*stride/scale;
        // float y3 = (output_buffer[basic_pos+5]+grid1)*stride/scale;
        // float x4 = (output_buffer[basic_pos+6]+grid0)*stride/scale;
        // float y4 = (output_buffer[basic_pos+7]+grid1)*stride/scale;


        ///置信度最大颜色
        //获取最大的颜色置信度索引
        int color_idx = argmax(output_buffer+basic_pos+class_start, params_.net_params.NUM_COLORS);
        //最大的颜色置信度的值
        float color_conf = output_buffer[basic_pos+class_start+color_idx];

        //置信度最大的类别
        //同上，获取类别索引
        int class_idx = argmax(output_buffer+basic_pos+class_start+params_.net_params.NUM_COLORS,
                                params_.net_params.NUM_CLASS);
        float class_conf = output_buffer[basic_pos+class_start+params_.net_params.NUM_COLORS+class_idx];

        //获取置信度

        // if(box_conf>prob_threshold){
        Object obj;

        obj.apexes = std::move(point);

        obj.rect = cv::boundingRect(obj.apexes);

        obj.label = class_idx;
        obj.color = color_idx;
        //分类置信度和识别置信度的乘积才是最后真正算出来的置信度
        obj.conf = box_conf*((class_conf+color_conf)/2);

        objects.emplace_back(obj);
        // }
    }
}

/**
 * @brief 对置信度递归进行一个快速排序
*/
void YoloxSmartModel::qsort_descent_inplace(std::vector<Object>& objects) {
    if(objects.empty()){
        return;
    }
    qsort_descent_inplace(objects, 0, objects.size()-1);
}

void YoloxSmartModel::qsort_descent_inplace(std::vector<Object> & faceobjects, int left, int right) {
    int i = left;
    int j = right;

    float p = faceobjects[(left+right)/2].conf;

    while(i<=j){
        while(faceobjects[i].conf>p){
            i++;
        }

        while(faceobjects[j].conf<p){
            j--;
        }

        if (i <= j) {
            std::swap(faceobjects[i], faceobjects[j]);

            i++;
            j--;
        }
    }

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            if(left<j)qsort_descent_inplace(faceobjects, left, j);
        }
        #pragma omp section
        {
            if(i<right)qsort_descent_inplace(faceobjects, i, right);
        }
    }
}

/**
 * @brief 非极大抑制
 * @param faceobjects 检测出来的结果
 * @param picked 非极大抑制后输出的索引就存放在这里
 * @param nms_threshold 非极大抑制阈值
*/
void YoloxSmartModel::nms_sorted_bboxes(std::vector<Object> & faceobjects, std::vector<int>& picked, float nms_threshold) {
    picked.clear();
    const int n = faceobjects.size();
    // std::vector<float> areas;
    // areas.reserve(n);

    // for(int i=0; i<n; i++){
    //     //计算每一个面积
    //     // std::vector<cv::Point2f> object_apex_tmp;
    //     // object_apex_tmp.emplace_back(faceobjects[i].p1);
    //     // object_apex_tmp.emplace_back(faceobjects[i].p2);
    //     // object_apex_tmp.emplace_back(faceobjects[i].p3);
    //     // object_apex_tmp.emplace_back(faceobjects[i].p4);
    //     // areas[i] = cv::contourArea(object_apex_tmp);
    //     // areas[i] = cv::contourArea(faceobjects[i].apexes);
    //     // areas[i] = faceobjects[i].rect.area();
    //     areas.emplace_back(faceobjects[i].rect.area());
    // }

    for(int i=0; i<n; i++){
        Object& a = faceobjects[i];
        // std::vector<cv::Point2f>apex_a(a.apexes);
        // apex_a.emplace_back(a.p1);
        // apex_a.emplace_back(a.p2);
        // apex_a.emplace_back(a.p3);
        // apex_a.emplace_back(a.p4);


        bool keep = true;

        for(int j=0; j<(int)picked.size(); j++){
            Object &b = faceobjects[picked[j]];
            
            // std::vector<cv::Point2f> apex_inter;
            // apex_b.emplace_back(b.p1);
            // apex_b.emplace_back(b.p2);
            // apex_b.emplace_back(b.p3);
            // apex_b.emplace_back(b.p4);

            // float inter_area = cv::intersectConvexConvex(a.apexes, b.apexes, apex_inter);
            float inter_area = intersaction_area(a, b);
            float union_area = a.rect.area() + b.rect.area() - inter_area;
            float iou = inter_area/union_area;

            if(iou>nms_threshold|| std::isnan(iou)){
                keep = false;

                if(iou>0.9 && abs(a.conf - b.conf) < 0.2&&a.label==b.label&&a.color==b.color){
                    // for (int k = 0; k < params_.net_params.NUM_APEX; k++) {
                    //     b.apexes.emplace_back(a.apexes[k]);
                    //     //areas[i] + areas[picked[j]] - inter_area
                    // }
                    b.apexes.insert(b.apexes.end(), a.apexes.begin(), a.apexes.end());

                }
            }
        }

        if(keep){
            picked.emplace_back(i);
        }
    }
}

/**
 * @brief 获取模型的输出后对结果进行解码
 * @param output_buffer 结果的首地址
 * @param object 解码后的结果保存在这里，具体看Object的定义
 * @param scale 输入图片对于原图片的缩放比例
 * 
*/
void YoloxSmartModel::decode(const float* output_buffer, std::vector<Object>& objects, float scale) {
    std::vector<Object>proposals;
    
    // std::vector<GridAndStride> grid_strides;

    generate_yolox_proposal(grid_strides_, output_buffer, params_.classifier_threshold, proposals, scale);

    // RCLCPP_INFO(rclcpp::get_logger("energy"), "aaaaaaa; %ld", proposals.size());

    qsort_descent_inplace(proposals);

    // if(proposals.size()>=128){
    //     proposals.resize(128);
    // }

    std::vector<int> picked;
    nms_sorted_bboxes(proposals, picked, params_.net_params.NMS_THRESH);

    int count = picked.size();
    step_ = count;
    //非极大抑制后的放入object里
    objects.reserve(count);
    for(int i=0; i<count; i++){
        objects.emplace_back(proposals[picked[i]]);
    }

    avg_rect(objects);
    // for (auto& object : objects) {
    //     auto N = object.apexes.size();
    //     if (N >= 2 * params_.net_params.NUM_APEX) {
    //         cv::Point2f fin_point[params_.net_params.NUM_APEX];

    //         for (int i = 0; i < N; i++) {
    //             fin_point[i % params_.net_params.NUM_APEX] += object.apexes[i];
    //         }

    //         for (int i = 0; i < params_.net_params.NUM_APEX; i++) {
    //             fin_point[i].x = fin_point[i].x / (N / params_.net_params.NUM_APEX);
    //             fin_point[i].y = fin_point[i].y / (N / params_.net_params.NUM_APEX);
    //         }
    //         object.apexes.clear();
    //         for (int i = 0; i < params_.net_params.NUM_APEX; i++) {
    //             object.apexes.emplace_back(fin_point[i]);
    //         } 
    //     }
    // }
}

void YoloxSmartModel::avg_rect(std::vector<Object>& objects) {
    for (auto& object : objects) {
        std::size_t N {object.apexes.size()};

        if (N >= 2 * params_.net_params.NUM_APEX) {
            cv::Point2f fin_point[params_.net_params.NUM_APEX];

            for (int i {0}; i < N; i++) {
                fin_point[i % params_.net_params.NUM_APEX] += object.apexes[i];
            }

            std::for_each(fin_point, fin_point + params_.net_params.NUM_APEX, [N, this](cv::Point2f& p) {
                p.x = p.x / (N / params_.net_params.NUM_APEX);
                p.y = p.y / (N / params_.net_params.NUM_APEX);
            });

            object.apexes.clear();
            std::for_each(fin_point, fin_point + params_.net_params.NUM_APEX, [&object](cv::Point2f& p) {
                object.apexes.emplace_back(p);
            });
        }
    }
}



float YoloxSmartModel::intersaction_area(const Object& a, const Object& b) {
    cv::Rect_<float> inter = a.rect & b.rect;
    return inter.area();
}

