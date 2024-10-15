#ifndef DRAWONPIC_H
#define DRAWONPIC_H
#include "Traditional.h"
#include <QLabel>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <QtSvg/QSvgRenderer>
#include "model.hpp"
#include "configure.hpp"
#include <QStack>
#include <QDateTime>
#include <fstream>
class LabelDialog;
#define NULL_IMG cv::Mat(0, 0, CV_8UC1)

enum LabelMode{
    Armor,
    Wind_Armor,
    Wind,
    Engineer

};
enum ModelMode {
    Yolov5,
    Yolox,
};

typedef struct BaseNetDetectorParams {
    int autoaim_mode;
    double classifier_threshold;
    double min_large_center_distance;
    
    struct NetParams {
        NetParams(
            const std::string& model_path,
            int num_class,
            int num_colors,
            float nms_thresh,
            int num_apex
        ) : MODEL_PATH(model_path),
            NUM_CLASS(num_class),
            NUM_COLORS(num_colors),
            NMS_THRESH(nms_thresh),
            NUM_APEX(num_apex){}
        
        NetParams() = default;
        std::string MODEL_PATH;
        int NUM_CLASS;
        int NUM_COLORS;
        float NMS_THRESH;
        int NUM_APEX;
    };
    
    NetParams net_params;
} BaseNetDetectorParams;

class BaseSmartModel {
public:
    virtual ~BaseSmartModel() = default;
    virtual bool run(const QString &image_file, QVector<box_t> &boxes) = 0;
    virtual QString get_mode() const = 0;
};



class Yolov5SmartModel : public BaseSmartModel {
public:
    explicit Yolov5SmartModel();
    bool run(const QString &image_file, QVector<box_t> &boxes) override;
    QString get_mode() const override{ return "YOLOv5";};

private:
    cv::dnn::Net net;
};


class YoloxSmartModel : public BaseSmartModel {
public:
    explicit YoloxSmartModel(int detect_mode);
    bool run(const QString &image_file, QVector<box_t> &boxes) override;
    QString get_mode() const override{ return "YOLOX";};
    BaseNetDetectorParams loadConfig(const std::string& config_path, int detect_mode);
private:
    cv::dnn::Net net_;
    std::shared_ptr<cv::Point2f> R_center;
    struct Object{//存储检测结果
        int label;//分类
        int color;//颜色（这俩都同上）
        float conf;//置信度
        // cv::Point2f p1, p2, p3, p4;//左上角开始逆时针四个点]
        std::vector<cv::Point2f> apexes;
        cv::Rect_<float> rect;//外接矩形，nms非极大抑制用
        
    };
    struct GridAndStride{//特征图大小和相对于输入图像的步长大小
        int grid0;
        int grid1;
        int stride;
    };

    int INPUT_W;
    int INPUT_H;
    BaseNetDetectorParams params_;
    float scale_;//输入大小（416*416）和原图长边的比例
    float scale_x_;
    float scale_y_;
    int detect_mode_;

    int argmax(const float* ptr, int len);
    cv::Mat static_resize(cv::Mat src);
    std::vector<GridAndStride> grid_strides_;
    int step_;
    



    void generate_grids_and_stride(const int w, const int h, const int strides[], std::vector<GridAndStride> &grid_strides);
    void generate_yolox_proposal(std::vector<GridAndStride> &grid_strides, const float * output_buffer, float prob_threshold, std::vector<Object>& object, float scale);
    void qsort_descent_inplace(std::vector<Object> & faceobjects, int left, int right);
    void qsort_descent_inplace(std::vector<Object>& objects);
    void nms_sorted_bboxes(std::vector<Object> & faceobjects, std::vector<int>& picked, float nms_threshold);
    void decode(const float* output_buffer, std::vector<Object>& object, float scale);

    float intersaction_area(const Object& a, const Object& b);

    void avg_rect(std::vector<Object>& objects);

};


class SmartModelFactory {
public:
    static std::unique_ptr<BaseSmartModel> createSmartModel(const QString &model_mode){
        if(model_mode == "YOLOv5"){
            return std::make_unique<Yolov5SmartModel>();
    }
        else if(model_mode == "YOLOX"){
            return std::make_unique<YoloxSmartModel>(0);
    }
         throw std::invalid_argument("Unknown model type");
    }
};



class DrawOnPic : public QLabel {
Q_OBJECT

public:
    void openLabelDialog();

    explicit DrawOnPic(QWidget *parent = nullptr);

   //QString model_mode() const { return model.get_mode(); }

    QString current_file;

    void reset();

    QVector<box_t> &get_current_label();

    void load_svg();

    cv::Mat modified_img = NULL_IMG, enh_img = NULL_IMG;
    bool image_equalizeHist = false;
    bool image_enhanceV = false;
    bool del_file = false;
    LabelMode label_mode = Armor;
    Configure configure;
    QTransform img2label;       // 图像坐标到实际显示的坐标
    void setTransform(const QTransform &transform) {
        img2label = transform;
        update();}
    QImage *img = nullptr;
        void setModelMode(const QString &modelMode) ;
protected:
    void mousePressEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);

    void mouseDoubleClickEvent(QMouseEvent *event);

    void wheelEvent(QWheelEvent *event);

    void keyPressEvent(QKeyEvent *event) override;

    void paintEvent(QPaintEvent *event);

    bool event(QEvent *event) override;


public slots:

    void setCurrentFile(QString file);

    void loadImage();

    void saveLabel();

    void setAddingMode();

    void setNormalMode();

    void setFocusBox(int index);

    void removeBox(box_t *box);

    void smart() ;

    void updateBox();

    void stayPositionChanged(bool value);

    void illuminate();

    void histogram_Equalization();

    void cover_brush();

    void verifyBoxes();


signals:

    void openLabelDialogRequested(QVector<box_t>::iterator box_iter);


    void labelChanged(const QVector<box_t> &);

    void delCurrentImage();

    void update_list_name_signal(const LabelMode mode);

    void labelDialogClosed(int boxIndex);
private:


    QVector<box_t> lastSmartLabels;
    struct BoxState {
        int boxIndex;
        int pointIndex;
        QPointF oldPosition;
    };
    QVector<BoxState> undoStack;

    void saveStateForUndo();

    LabelDialog* currentLabelDialog = nullptr;
    void loadLabel();

    void update_cover(QPointF center);

    void drawROI(QPainter &painter);

    void resizeEvent(QResizeEvent *event);
    QPointF *checkPoint();
    double currentScale;
    void updateTransform();
    int label_to_size(int label, LabelMode mode);

private:
    std::unique_ptr<BaseSmartModel> currentModel;
    QSvgRenderer standard_tag_render[12];

    //SmartModel model;

    QTransform norm2img;        // 归一化图像坐标到图像坐标
    //QTransform img2label;       // 图像坐标到实际显示的坐标

    bool stayPosition = false;   //为true时加载图像时不刷新img2label（即继续显示同一局部

    // double ratio;
    // int dx, dy;
    //QImage *img = nullptr;

    cv::Mat showing_modified_img;

    QPolygonF big_svg_ploygen, small_svg_ploygen;
    QPolygonF big_pts, small_pts;

    QVector<box_t> current_label;   // 归一化坐标

    QPointF *draging = nullptr;
    int cover_radius = 10;
    int focus_box_index = -1;
    int focus_point_index = -1;
    int banned_point_index = -1;
    bool F_mode = false;
    QVector<QPointF> adding;
    QPointF pos;

    QPointF drag_start_pos;

    QPen pen_point_focus;
    QPen pen_point;
    QPen pen_box_focus;
    QPen pen_box;
    QPen pen_line;
    QPen pen_text;

    int latency_ms = -1;

    enum mode_t {
        NORMAL_MODE,
        ADDING_MODE,
        COVER_MODE,
        VERIFY_MODE
    } mode = NORMAL_MODE;

    bool showSvg = true;
    QPointF transformedCorners[4];
    QImage transformedImage;
    QTransform svg2painter;
    int label_to_size(int label, LabelMode mode) const;
    bool is_big(const box_t& box) const;
    // void performTransformation(const box_t& box);
    // QPointF calculateCenter(const QVector<QPointF>& points);
    // double calculateArea(const QVector<QPointF>& points);
    // QTransform calculateNormalizingTransform(const QVector<QPointF>& points);
    // void saveTransformedImage(const QImage& originalImage, const QVector<QPointF>& points, const QString& filename);
};


#endif // DRAWONPIC_H
