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
class DrawOnPic : public QLabel {
Q_OBJECT

public:
    void openLabelDialog();

    explicit DrawOnPic(QWidget *parent = nullptr);

    QString model_mode() const { return model.get_mode(); }

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

    void smart();

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

    QSvgRenderer standard_tag_render[12];

    SmartModel model;

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
