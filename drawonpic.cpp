#include "drawonpic.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QTransform>
#include <QDebug>
#include <iostream>
#include <fstream>
#include <chrono>
#include <QClipboard>
#include <QMimeData>
#include <filesystem>
#include <regex>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include "mainwindow.h"
#include "labeldialog.h"
#include <QtMath>
#include "Traditional.h"
//DrawOnPic::DrawOnPic(QWidget *parent) : QLabel(parent), model() {
DrawOnPic::DrawOnPic(QWidget *parent) : QLabel(parent) {
    pen_point_focus.setWidth(5);
    pen_point_focus.setColor(Qt::green);

    pen_point.setWidth(5);
    pen_point.setColor(Qt::green);

    pen_line.setWidth(1);
    pen_line.setColor(Qt::green);

    pen_text.setWidth(4);
    pen_text.setColor(Qt::green);

    pen_box.setWidth(2);
    pen_box.setColor(Qt::green);

    pen_box_focus.setWidth(3);
    pen_box_focus.setColor(Qt::red);

    // // 大装甲svg宽高
    // big_svg_ploygen.append({0., 0.});
    // big_svg_ploygen.append({0., 478.});
    // big_svg_ploygen.append({871., 478.});
    // big_svg_ploygen.append({871., 0.});
    // // 小装甲svg宽高
    // small_svg_ploygen.append({0., 0.});
    // small_svg_ploygen.append({0., 516.});
    // small_svg_ploygen.append({557., 516.});
    // small_svg_ploygen.append({557., 0.});

// 重新测量后的坐标点定义
    // 大装甲标注点对应在svg图中的4个坐标
    // big_pts.append({0., 140.61});
    // big_pts.append({0., 347.39});
    // big_pts.append({871., 347.39});
    // big_pts.append({871., 140.61});
    // // 小装甲标注点对应在svg图中的4个坐标
    // small_pts.append({0., 143.26});
    // small_pts.append({0., 372.74});
    // small_pts.append({557., 372.74});
    // small_pts.append({557., 143.26});

    // 历史坐标点定义
//     // 大装甲标注点对应在svg图中的4个坐标
//     big_pts.append({11., 141.});
//     big_pts.append({11., 344.});
//     big_pts.append({860., 344.});
//     big_pts.append({860., 141.});
//     // 小装甲标注点对应在svg图中的4个坐标
//     small_pts.append({11., 146.});
//     small_pts.append({11., 371.});
//     small_pts.append({546., 371.});
//     small_pts.append({546., 146.});

    // 加载svg图片
    // standard_tag_render[0].load(QString(":/pic/tags/resource/G.svg"));
    // standard_tag_render[1].load(QString(":/pic/tags/resource/1.svg"));
    // standard_tag_render[2].load(QString(":/pic/tags/resource/2.svg"));
    // standard_tag_render[3].load(QString(":/pic/tags/resource/3.svg"));
    // standard_tag_render[4].load(QString(":/pic/tags/resource/4.svg"));
    // standard_tag_render[5].load(QString(":/pic/tags/resource/5.svg"));
    // standard_tag_render[6].load(QString(":/pic/tags/resource/O.svg"));
    // standard_tag_render[7].load(QString(":/pic/tags/resource/Bs.svg"));
    // standard_tag_render[8].load(QString(":/pic/tags/resource/Bb.svg"));
    // standard_tag_render[9].load(QString(":/pic/tags/resource/B3.svg"));
    // standard_tag_render[10].load(QString(":/pic/tags/resource/B4.svg"));
    // standard_tag_render[11].load(QString(":/pic/tags/resource/B5.svg"));
    load_svg();

    // 读取鼠标键盘的数据
    this->setMouseTracking(true);
    this->grabKeyboard();

}

void DrawOnPic::mousePressEvent(QMouseEvent *event) {
    QPointF p;
    pos = event->pos(); // 记录当前鼠标位置

    if (event->button() == Qt::LeftButton) {
        switch (mode) {
            case NORMAL_MODE:
                // normal模式下，鼠标左键用于确定兴趣点。
                // 用于拖动定位点。
                draging = checkPoint();
                if (draging) {
                    for (int i = 0; i < current_label.size(); ++i) {
                        for (int j = 0; j < 4; ++j) {
                            if (draging == current_label[i].pts + j) {
                                focus_box_index = i;
                                focus_point_index = j;
                                // 记录初始状态
                                undoStack.push_back({i, j, *draging});
                                break;
                            }
                        }
                    }
                }
                break;
            case ADDING_MODE:
                break;
            case COVER_MODE:
                if (!modified_img.rows)
                    modified_img = cv::imread(current_file.toStdString());
                p = img2label.inverted().map(pos);
                if (p.x() >= 0 && p.y() >= 0 && p.x() <= img->width() && p.y() <= img->height()) {
                    cv::circle(modified_img, cv::Point2f(p.x(), p.y()), cover_radius, 0, -1);
                    if (image_equalizeHist + image_enhanceV) {
                        cv::circle(enh_img, cv::Point2f(p.x(), p.y()), cover_radius, 0, -1);
                        img->operator=(
                                QImage((const unsigned char *) enh_img.data, enh_img.cols, enh_img.rows, enh_img.step,
                                       QImage::Format_RGB888));
                    } else {
                        cv::cvtColor(modified_img, showing_modified_img, cv::COLOR_RGB2BGR);
                        img->operator=(
                                QImage((const unsigned char *) showing_modified_img.data, showing_modified_img.cols,
                                       showing_modified_img.rows, showing_modified_img.step, QImage::Format_RGB888));
                    }
                    update();
                }
                break;
            default:
                break;
        }
        update();   // 更新绘图
    }     else if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton) {
    // 右键或中键用于图像拖动，记录拖动起点
    drag_start_pos = pos;
    setNormalMode();
    update();   // 更新绘图
    }
}

void DrawOnPic::mouseMoveEvent(QMouseEvent *event) {
    pos = event->pos();

    switch (mode) {
        case NORMAL_MODE:
            if (draging) {
                *draging = norm2img.inverted().map(img2label.inverted().map(pos));
                if (F_mode) {
                    if (focus_box_index != -1) {
                        if (current_label[focus_box_index].pts[0].x() == current_label[focus_box_index].pts[1].x()) {
                            current_label[focus_box_index].pts[2].setX(current_label[focus_box_index].pts[3].x());
                        } else {
                            double k = (current_label[focus_box_index].pts[0].y() -
                                        current_label[focus_box_index].pts[1].y()) /
                                       (current_label[focus_box_index].pts[0].x() -
                                        current_label[focus_box_index].pts[1].x());
                            double b = current_label[focus_box_index].pts[3].y() -
                                       k * current_label[focus_box_index].pts[3].x();
                            current_label[focus_box_index].pts[2].setX(
                                    (current_label[focus_box_index].pts[2].y() - b) / k);
                        }
                    }
                } else if (banned_point_index != -1 && draging !=
                                                       &current_label[focus_box_index].pts[banned_point_index]) { // 如果正在拖动一个定位点，则将定位点位置设置成鼠标位置
                    *draging = norm2img.inverted().map(img2label.inverted().map(pos));
                    if (current_label[focus_box_index].pts[(banned_point_index + 2) % 4].x() ==
                        current_label[focus_box_index].pts[(banned_point_index + 3) % 4].x()) {
                        current_label[focus_box_index].pts[banned_point_index].setX(
                                current_label[focus_box_index].pts[(banned_point_index + 1) % 4].x());
                        current_label[focus_box_index].pts[banned_point_index].setY(
                                current_label[focus_box_index].pts[(banned_point_index + 1) % 4].y() -
                                current_label[focus_box_index].pts[(banned_point_index + 2) % 4].y() +
                                current_label[focus_box_index].pts[(banned_point_index + 3) % 4].y());
                    } else {
                        double k = (current_label[focus_box_index].pts[(banned_point_index + 2) % 4].y() -
                                    current_label[focus_box_index].pts[(banned_point_index + 3) % 4].y()) /
                                   (current_label[focus_box_index].pts[(banned_point_index + 2) % 4].x() -
                                    current_label[focus_box_index].pts[(banned_point_index + 3) % 4].x());
                        double b = current_label[focus_box_index].pts[(banned_point_index + 1) % 4].y() -
                                   k * current_label[focus_box_index].pts[(banned_point_index + 1) % 4].x();
                        if (banned_point_index % 2) {
                            current_label[focus_box_index].pts[banned_point_index].setX(
                                    current_label[focus_box_index].pts[(banned_point_index + 1) % 4].x() -
                                    current_label[focus_box_index].pts[(banned_point_index + 2) % 4].x() +
                                    current_label[focus_box_index].pts[(banned_point_index + 3) % 4].x());
                            current_label[focus_box_index].pts[banned_point_index].setY(
                                    k * current_label[focus_box_index].pts[banned_point_index].x() + b);
                        } else {
                            current_label[focus_box_index].pts[banned_point_index].setY(
                                    current_label[focus_box_index].pts[(banned_point_index + 1) % 4].y() -
                                    current_label[focus_box_index].pts[(banned_point_index + 2) % 4].y() +
                                    current_label[focus_box_index].pts[(banned_point_index + 3) % 4].y());
                            current_label[focus_box_index].pts[banned_point_index].setX(
                                    (current_label[focus_box_index].pts[banned_point_index].y() - b) / k);
                        }
                    }
                }
            }
            update();
            break;
        case ADDING_MODE:
            update();
            break;
        case COVER_MODE:
            update_cover(pos);
            break;
        default:
            break;
    }

    // 右键拖动，计算图片移动后的QTransform
/*    if (event->buttons() & Qt::RightButton|Qt::MiddleButton) {
        QTransform delta;
        delta.translate(pos.x() - right_drag_pos.x(), pos.y() - right_drag_pos.y());
        img2label = img2label * delta;
        right_drag_pos = pos;
        update();
    } else if (event->buttons() & Qt::MiddleButton) {
        if (focus_box_index == -1) return;
        auto new_pos = norm2img.inverted().map(img2label.inverted().map(pos));
        for (auto &pt: current_label[focus_box_index].pts) {
            pt += new_pos - middle_drag_pos;
        }
        middle_drag_pos = new_pos;
        update();
    } */// 右键或中键拖动，计算图片移动后的QTransform
    if (event->buttons() & (Qt::RightButton | Qt::MiddleButton)) {
        QTransform delta;
        delta.translate(pos.x() - drag_start_pos.x(), pos.y() - drag_start_pos.y());
        img2label = img2label * delta;
        drag_start_pos = pos;
        update();
    }else if ((event->buttons() & Qt::LeftButton) && mode == COVER_MODE) {
        QPointF p = img2label.inverted().map(pos);
        if (p.x() >= 0 && p.y() >= 0 && p.x() <= img->width() && p.y() <= img->height()) {
            cv::circle(modified_img, cv::Point2f(p.x(), p.y()), cover_radius, 0, -1);
            if (image_equalizeHist + image_enhanceV) {
                cv::circle(enh_img, cv::Point2f(p.x(), p.y()), cover_radius, 0, -1);
                img->operator=(QImage((const unsigned char *) enh_img.data, enh_img.cols, enh_img.rows, enh_img.step,
                                      QImage::Format_RGB888));
            } else {
                cv::cvtColor(modified_img, showing_modified_img, cv::COLOR_RGB2BGR);
                img->operator=(QImage((const unsigned char *) showing_modified_img.data, showing_modified_img.cols,
                                      showing_modified_img.rows, showing_modified_img.step, QImage::Format_RGB888));
            }
            update();
        }
    }
}
// void DrawOnPic::performTransformation(const box_t& box) {
//     // Calculate the bounding rectangle for the label points
//     QPolygonF labelPolygon;
//     for (int i = 0; i < 4; ++i) {
//         labelPolygon.append(norm2img.map(box.pts[i]));
//     }
//     QRect boundingRect = labelPolygon.boundingRect().toRect();

//     // Add some margin to the bounding rectangle
//     int margin = 10;
//     boundingRect.adjust(-margin, -margin, margin, margin);
//     boundingRect = boundingRect.intersected(img->rect());

//     // Crop the original image
//     QImage croppedImage = img->copy(boundingRect);

//     // Calculate the transformation from SVG to the cropped image
//     QPolygonF svgPolygon = is_big(box) ? big_pts : small_pts;
//     QPolygonF croppedPolygon;
//     for (int i = 0; i < 4; ++i) {
//         croppedPolygon.append(norm2img.map(box.pts[i]) - boundingRect.topLeft());
//     }

//     QTransform transform;
//     QTransform::quadToQuad(svgPolygon, croppedPolygon, transform);

//     // Create and save the cropped image
//     QString croppedFilename = QString("cropped_%1.png").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
//     croppedImage.save(croppedFilename);

//     // Create the final transformed image
//     QImage transformedImage(croppedImage.size(), QImage::Format_ARGB32_Premultiplied);
//     transformedImage.fill(Qt::transparent);

//     QPainter painter(&transformedImage);
//     painter.setRenderHint(QPainter::Antialiasing);
//     painter.setTransform(transform);
//     standard_tag_render[box.tag_id].render(&painter);

//     // Save the final transformed image
//     QString transformedFilename = QString("transformed_%1.png").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
//     transformedImage.save(transformedFilename);

//     // Calculate and save the coordinates of the original points in both cropped and transformed images
//     QFile file(transformedFilename + ".txt");
//     if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
//         QTextStream stream(&file);
//         stream << "Crop rect: " << boundingRect.x() << " " << boundingRect.y() << " "
//                << boundingRect.width() << " " << boundingRect.height() << "\n";
//         stream << "Points in cropped image:\n";
//         for (int i = 0; i < 4; ++i) {
//             QPointF croppedPoint = norm2img.map(box.pts[i]) - boundingRect.topLeft();
//             stream << croppedPoint.x() << " " << croppedPoint.y() << "\n";
//         }
//         stream << "Points in transformed image:\n";
//         for (int i = 0; i < 4; ++i) {
//             QPointF croppedPoint = norm2img.map(box.pts[i]) - boundingRect.topLeft();
//             QPointF transformedPoint = transform.map(croppedPoint);
//             stream << transformedPoint.x() << " " << transformedPoint.y() << "\n";
//         }
//     }

//     // Print the coordinates to console for debugging
//     qDebug() << "Crop rect:" << boundingRect;
//     qDebug() << "Points in cropped image:";
//     for (int i = 0; i < 4; ++i) {
//         QPointF croppedPoint = norm2img.map(box.pts[i]) - boundingRect.topLeft();
//         qDebug() << "Point" << i << ":" << croppedPoint;
//     }
//     qDebug() << "Points in transformed image:";
//     for (int i = 0; i < 4; ++i) {
//         QPointF croppedPoint = norm2img.map(box.pts[i]) - boundingRect.topLeft();
//         QPointF transformedPoint = transform.map(croppedPoint);
//         qDebug() << "Point" << i << ":" << transformedPoint;
//     }
// }
// QPointF DrawOnPic::calculateCenter(const QVector<QPointF>& points) {
//     QPointF center(0, 0);
//     for (const auto& point : points) {
//         center += point;
//     }
//     return center / points.size();
// }

// double DrawOnPic::calculateArea(const QVector<QPointF>& points) {
//     double area = 0.0;
//     for (int i = 0; i < points.size(); i++) {
//         int j = (i + 1) % points.size();
//         area += (points[i].x() * points[j].y() - points[j].x() * points[i].y());
//     }
//     return qAbs(area) / 2.0;
// }

// QTransform DrawOnPic::calculateNormalizingTransform(const QVector<QPointF>& points) {
//     // 确保我们有四个点
//     if (points.size() != 4) {
//         qDebug() << "Error: Expecting 4 points, got" << points.size();
//         return QTransform();
//     }

//     // 找出左上角点（x和y坐标之和最小的点）
//     int topLeftIndex = 0;
//     double minSum = points[0].x() + points[0].y();
//     for (int i = 1; i < 4; ++i) {
//         double sum = points[i].x() + points[i].y();
//         if (sum < minSum) {
//             minSum = sum;
//             topLeftIndex = i;
//         }
//     }

//     // 重新排序点，使左上角点为第一个
//     QVector<QPointF> orderedPoints;
//     for (int i = 0; i < 4; ++i) {
//         orderedPoints.append(points[(topLeftIndex + i) % 4]);
//     }

//     QPointF topLeft = orderedPoints[0];
//     QPointF topRight = orderedPoints[1];
//     QPointF bottomRight = orderedPoints[2];
//     QPointF bottomLeft = orderedPoints[3];

//     // 计算新的宽度和高度
//     double width = qMax(QLineF(topLeft, topRight).length(), QLineF(bottomLeft, bottomRight).length());
//     double height = qMax(QLineF(topLeft, bottomLeft).length(), QLineF(topRight, bottomRight).length());

//     // 创建源多边形和目标多边形
//     QPolygonF src;
//     src << topLeft << topRight << bottomRight << bottomLeft;

//     QPolygonF dst;
//     dst << QPointF(0, 0) << QPointF(width, 0) << QPointF(width, height) << QPointF(0, height);

//     // 计算变换
//     QTransform transform;
//     QTransform::quadToQuad(src, dst, transform);

//     qDebug() << "Normalizing transform:";
//     qDebug() << transform;

//     return transform;
// }

// void DrawOnPic::saveTransformedImage(const QImage& originalImage, const QVector<QPointF>& normalizedPoints, const QString& filename) {
//     // 将归一化坐标转换为像素坐标
//     QVector<QPointF> pixelPoints;
//     for (const auto& point : normalizedPoints) {
//         pixelPoints.append(QPointF(point.x() * originalImage.width(), point.y() * originalImage.height()));
//     }

//     // 计算包围所有点的矩形
//     qreal minX = std::numeric_limits<qreal>::max();
//     qreal minY = std::numeric_limits<qreal>::max();
//     qreal maxX = std::numeric_limits<qreal>::lowest();
//     qreal maxY = std::numeric_limits<qreal>::lowest();

//     for (const auto& point : pixelPoints) {
//         minX = qMin(minX, point.x());
//         minY = qMin(minY, point.y());
//         maxX = qMax(maxX, point.x());
//         maxY = qMax(maxY, point.y());
//     }

//     // 扩大矩形以包含所有点（添加一些边距）
//     int margin = 10; // 可以根据需要调整
//     QRect cropRect(QPoint(qFloor(minX) - margin, qFloor(minY) - margin),
//                    QPoint(qCeil(maxX) + margin, qCeil(maxY) + margin));

//     // 确保cropRect在原图范围内
//     cropRect = cropRect.intersected(originalImage.rect());

//     // 裁剪原图
//     QImage croppedImage = originalImage.copy(cropRect);

//     // 计算裁剪后的点坐标
//     QVector<QPointF> croppedPoints;
//     for (const auto& point : pixelPoints) {
//         croppedPoints.append(point - cropRect.topLeft());
//     }

//     // 保存裁剪后的图像
//     croppedImage.save(filename);

//     // 保存原始点在裁剪图像中的坐标
//     QFile coordFile(filename + "_coordinates.txt");
//     if (coordFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
//         QTextStream stream(&coordFile);
//         for (const auto& point : croppedPoints) {
//             stream << qRound(point.x()) << " " << qRound(point.y()) << "\n";
//         }
//     }


//     qDebug() << "Original normalized points:";
//     for (const auto& point : normalizedPoints) {
//         qDebug() << point;
//     }
//     qDebug() << "Pixel points in original image:";
//     for (const auto& point : pixelPoints) {
//         qDebug() << point;
//     }
//     qDebug() << "Points in cropped image:";
//     for (const auto& point : croppedPoints) {
//         qDebug() << point;
//     }
//     qDebug() << "Crop rect:" << cropRect;
//     qDebug() << "Cropped image size:" << croppedImage.width() << "x" << croppedImage.height();
// }
int DrawOnPic::label_to_size(int label, LabelMode mode) const  // Add 'const' here
{
    if(mode == Armor)
        return label == 1 || label > 7;
    else if(mode == Engineer)
        return label == 4;
    else
        return 0;
}

bool DrawOnPic::is_big(const box_t& box) const {
    return label_to_size(box.tag_id, label_mode);
}
void DrawOnPic::mouseReleaseEvent(QMouseEvent *event) {
    pos = event->pos();
    if (event->button() == Qt::LeftButton) {
        switch (mode) {
        case NORMAL_MODE:   // 松开左键，停止拖动定位点
            draging = nullptr;
            break;
case ADDING_MODE:
            adding.append(norm2img.inverted().map(img2label.inverted().map(pos)));
            if (adding.size() == 4 + (label_mode == Wind)) {
                box_t box;
                for (int i = 0; i < 4 + (label_mode == Wind); ++i) box.pts[i] = adding[i];

                // 使用 TraditionalDetector 检测装甲板
                cv::Mat src = cv::imread(current_file.toStdString());
                if (src.empty()) {
                    qDebug() << "Error: Unable to read image file: " << current_file;
                    break;
                }

                std::vector<cv::Point2f> points;
                for (const auto& pt : adding) {
                    points.push_back(cv::Point2f(pt.x() * src.cols, pt.y() * src.rows));
                }

                TraditionalDetector detector;
                DetectionResult result = detector.detectAndClassify(src, points);

                // 设置颜色和tag_id
                box.color_id = result.color_id;
                box.tag_id = result.tag_id;

                current_label.append(box);
                setNormalMode();
                emit labelChanged(current_label);

                // 发送新标签添加的信号
                emit newLabelAdded(current_label.size() - 1);
            }
            update();
            break;
        default:
            break;
        }
    }
}
void DrawOnPic::verifyBoxes() {
    if (current_file.isEmpty() || current_label.empty()) return;

    TraditionalDetector detector;
    cv::Mat src = cv::imread(current_file.toStdString());
    if (src.empty()) {
        qDebug() << "Error: Unable to read image file:" << current_file;
        return;
    }

    for (auto &box : current_label) {
        std::vector<cv::Point2f> points;
        for (const auto &pt : box.pts) {
            points.push_back(cv::Point2f(pt.x() * src.cols, pt.y() * src.rows));
        }

        DetectionResult result = detector.detectAndClassify(src, points);

        if (result.confidence > 0.5) {
            box.color_id = result.color_id;

            if (result.number == "outpost") {
                box.tag_id = 6;
            } else if (result.number == "base") {
                box.tag_id = result.type == Armor::Type::SMALL ? 7 : 8;
            } else if (result.number == "guard") {
                box.tag_id = 0;
            } else {
                int detectedNumber = -1;
                bool isNumber = false;
                try {
                    detectedNumber = std::stoi(result.number);
                    isNumber = true;
                } catch (const std::exception& e) {
                    qDebug() << "Failed to convert number:" << QString::fromStdString(result.number);
                }

                // if (isNumber) {
                //     if (detectedNumber == 1 || detectedNumber == 2) {
                //         box.tag_id = detectedNumber;
                //     } else if (detectedNumber >= 3 && detectedNumber <= 5) {
                //         if (result.type == Armor::Type::SMALL) {
                //             box.tag_id = detectedNumber + 1; // 3, 4, 5 -> 4, 5, 6
                //         } else {
                //             box.tag_id = detectedNumber + 6; // 3, 4, 5 -> 9, 10, 11
                //         }
                //     } else {
                //         qDebug() << "Warning: Unrecognized armor number:" << detectedNumber;

                //     }
                if (isNumber) {
                     box.tag_id = detectedNumber;

                } else {
                    qDebug() << "Warning: Invalid armor number:" << QString::fromStdString(result.number);

                }
            }

            qDebug() << "Verified box:";
            qDebug() << "Number:" << QString::fromStdString(result.number);
            qDebug() << "Confidence:" << result.confidence;
            qDebug() << "Type:" << (result.type == Armor::Type::SMALL ? "SMALL" : "LARGE");
            qDebug() << "Color:" << QString::fromStdString(TraditionalDetector::colorToString(result.color));
            qDebug() << "Assigned tag_id:" << box.tag_id;
        } else {
            qDebug() << "TraditionalDetector couldn't confidently classify a box. Keeping original results.";
        }
    }

    emit labelChanged(current_label);
    update();
}
void DrawOnPic::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        // 右键双击恢复默认视图
        // 偷懒，使用重新加载图像实现上述功能
        loadImage();
    }
}

void DrawOnPic::wheelEvent(QWheelEvent *event) {
    if (!img) return;  // 确保图像已加载
    const double delta = (event->delta() > 0) ? 1.1 : 1 / 1.1;

    // 获取当前显示的图像中心
    QPointF center = QPointF(this->width() / 2, this->height() / 2);

    // 计算当前图像的显示大小
    QRectF currentRect = img2label.mapRect(QRectF(0, 0, img->width(), img->height()));

    // 计算最小缩放比例（确保最长边覆盖整个窗口）
    double scaleX = this->width() / (double)img->width();
    double scaleY = this->height() / (double)img->height();
    double minScale = qMin(scaleX, scaleY);  // 使用qMin而不是qMax

    // 计算当前的缩放比例
    double currentScale = qMin(currentRect.width() / img->width(), currentRect.height() / img->height());
    qDebug() << "Before scaling - Current scale:" << currentScale << "Min scale:" << minScale;

    // 如果正在缩小并且已经接近或达到最小尺寸，直接设置为最小尺寸
    if (delta < 1 && qAbs(currentScale - minScale) < 0.01) {
        QTransform fitTransform;
        fitTransform.scale(minScale, minScale);
        fitTransform.translate(-img->width()/2, -img->height()/2);
        fitTransform.translate(this->width()/(2*minScale), this->height()/(2*minScale));
        img2label = fitTransform;
        qDebug() << "Set to min scale:" << minScale;
        update();
        return;
    }

    QTransform delta_transform;
    if (delta > 1) {
        // 放大操作：以鼠标位置为中心
        double mx = event->pos().x();
        double my = event->pos().y();
        delta_transform.translate(mx * (1 - delta), my * (1 - delta)).scale(delta, delta);
    } else {
        // 缩小操作：以图像中心为基准
        delta_transform.translate(center.x() * (1 - delta), center.y() * (1 - delta)).scale(delta, delta);
    }

    // 应用变换
    QTransform newTransform = img2label * delta_transform;

    // 检查新的变换是否会导致图像小于最小缩放比例
    QRectF newRect = newTransform.mapRect(QRectF(0, 0, img->width(), img->height()));
    double newScale = qMin(newRect.width() / img->width(), newRect.height() / img->height());

    if (newScale >= minScale) {
        // 只有在缩放后图像仍然大于或等于最小缩放比例时才应用新的变换
        img2label = newTransform;
        qDebug() << "After scaling - New scale:" << newScale;
    } else {
        // 如果低于最小缩放比例，则设置为最小缩放
        QTransform minTransform;
        minTransform.scale(minScale, minScale);
        minTransform.translate(-img->width()/2, -img->height()/2);
        minTransform.translate(this->width()/(2*minScale), this->height()/(2*minScale));
        img2label = minTransform;
        qDebug() << "Set to min scale:" << minScale;
    }

    update(); // 确保重绘
}

void DrawOnPic::openLabelDialog() {
    qDebug() << "Opening label dialog";
    if (focus_box_index >= 0 && focus_box_index < current_label.size()) {
        if (currentLabelDialog) {
            currentLabelDialog->close();
            delete currentLabelDialog;
        }
        currentLabelDialog = new LabelDialog(&current_label[focus_box_index], label_mode);
        connect(currentLabelDialog, &LabelDialog::removeBoxEvent, this, &DrawOnPic::removeBox);
        connect(currentLabelDialog, &LabelDialog::changeBoxEvent, this, &DrawOnPic::updateBox);
        connect(currentLabelDialog, &QDialog::finished, this, [this](int result) {
            if (result == QDialog::Accepted) {
                emit labelChanged(current_label);
                update();
            }
            emit labelDialogClosed(focus_box_index);  // 发送信号，包含当前编辑的框的索引
            currentLabelDialog = nullptr;
        });
        currentLabelDialog->activateAndFocus();
    } else {
        qDebug() << "No box selected for editing";
    }
}

bool DrawOnPic::event(QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        keyPressEvent(static_cast<QKeyEvent*>(event));
        return true;
    }
    return QLabel::event(event);
}

void DrawOnPic::keyPressEvent(QKeyEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        switch (event->key()) {
        case Qt::Key_Z:  // Ctrl+Z: 撤销
            if (!undoStack.isEmpty()) {
                BoxState lastState = undoStack.takeLast();
                current_label[lastState.boxIndex].pts[lastState.pointIndex] = lastState.oldPosition;
                update();
                emit labelChanged(current_label);
            } else if (!lastSmartLabels.isEmpty()) {
                // 如果没有点的修改可以撤销，但有上一次的智能标注，则撤销智能标注
                current_label = lastSmartLabels;
                lastSmartLabels.clear();
                update();
                emit labelChanged(current_label);
            }

        // case Qt::Key_C:  // 保留原有的 Ctrl+C 功能
        //     if (focus_box_index >= 0) {
        //         auto box_data = QByteArray((char *) &current_label[focus_box_index], sizeof(box_t));
        //         auto *mime_data = new QMimeData();
        //         mime_data->setData("box_t", box_data);
        //         QApplication::clipboard()->setMimeData(mime_data);
        //     }
        //     return;
        // case Qt::Key_V:  // 保留原有的 Ctrl+V 功能
        // {
        //     auto mime_data = QApplication::clipboard()->mimeData();
        //     if (mime_data->hasFormat("box_t")) {
        //         auto box_to_paste = *(box_t *) mime_data->data("box_t").data();
        //         current_label.append(box_to_paste);
        //         focus_box_index = current_label.count() - 1;
        //         emit labelChanged(current_label);
        //         update();
        //     }
        // }
            return;
        }
    }
    switch (event->key()) {
        case Qt::Key_Escape:
            if (currentLabelDialog) {
                currentLabelDialog->close();
                currentLabelDialog = nullptr;
            } else {
                // 原有的 ESC 键处理逻辑
                focus_box_index = -1;
                focus_point_index = -1;
                banned_point_index = -1;
                update();
            }
            break;
        case Qt::Key_Delete: // Delete删除选中
            if (focus_box_index >= 0) {
                current_label.removeAt(focus_box_index);
                focus_box_index = -1;
                focus_point_index = -1;
                banned_point_index = -1;
                emit labelChanged(current_label);
                update();
            }
            break;
        case Qt::Key_D:
            qDebug() << "D key pressed in DrawOnPic";
            openLabelDialog();
            break;


#pragma region 复制粘贴
        // case Qt::Key_C: // Ctrl+C复制选中
        //     if (focus_box_index >= 0 && event->modifiers().testFlag(Qt::ControlModifier)) {
        //         auto box_data = QByteArray((char *) &current_label[focus_box_index], sizeof(box_t));
        //         auto *mime_data = new QMimeData();
        //         mime_data->setData("box_t", box_data);
        //         QApplication::clipboard()->setMimeData(mime_data);
        //     }

#pragma endregion
        case Qt::Key_V:
                    if (mode != VERIFY_MODE) {
                        mode = VERIFY_MODE;
                        verifyBoxes();
                        mode = NORMAL_MODE;
                    } else {
                        mode = NORMAL_MODE;
                    }
        update();
        break;
#pragma region 切换移动模式
        case Qt::Key_G: // 锚点平行模式
            if (focus_box_index != -1) {
                double k = (current_label[focus_box_index].pts[focus_point_index].y() -
                            current_label[focus_box_index].pts[(focus_point_index + 1) % 4].y()) /
                           (current_label[focus_box_index].pts[focus_point_index].x() -
                            current_label[focus_box_index].pts[(focus_point_index + 1) % 4].x());
                double b = current_label[focus_box_index].pts[(focus_point_index + 3) % 4].y() -
                           k * current_label[focus_box_index].pts[(focus_point_index + 3) % 4].x();
                if (focus_point_index % 2) {
                    current_label[focus_box_index].pts[(focus_point_index + 2) % 4].setY(
                            k * current_label[focus_box_index].pts[(focus_point_index + 2) % 4].x() + b);
                } else {
                    current_label[focus_box_index].pts[(focus_point_index + 2) % 4].setX(
                            (current_label[focus_box_index].pts[(focus_point_index + 2) % 4].y() - b) / k);
                }
                update();
            }
            break;
        case Qt::Key_F: // 定点平行模式
            banned_point_index = -1;
            if (!F_mode) {
                F_mode = true;
                if (focus_box_index != -1) {
                    if (current_label[focus_box_index].pts[0].x() == current_label[focus_box_index].pts[1].x()) {
                        current_label[focus_box_index].pts[2].setX(current_label[focus_box_index].pts[3].x());
                    } else {
                        double k = (current_label[focus_box_index].pts[0].y() -
                                    current_label[focus_box_index].pts[1].y()) /
                                   (current_label[focus_box_index].pts[0].x() -
                                    current_label[focus_box_index].pts[1].x());
                        double b = current_label[focus_box_index].pts[3].y() -
                                   k * current_label[focus_box_index].pts[3].x();
                        current_label[focus_box_index].pts[2].setX((current_label[focus_box_index].pts[2].y() - b) / k);
                    }
                    update();
                }
            } else {
                F_mode = false;
            }
            break;
        case Qt::Key_H: // 等长锚点平行
            if (focus_box_index != -1) {
                double k = (current_label[focus_box_index].pts[focus_point_index].y() -
                            current_label[focus_box_index].pts[(focus_point_index + 1) % 4].y()) /
                           (current_label[focus_box_index].pts[focus_point_index].x() -
                            current_label[focus_box_index].pts[(focus_point_index + 1) % 4].x());
                double b = current_label[focus_box_index].pts[(focus_point_index + 3) % 4].y() -
                           k * current_label[focus_box_index].pts[(focus_point_index + 3) % 4].x();
                if (focus_point_index % 2) {
                    current_label[focus_box_index].pts[(focus_point_index + 2) % 4].setX(
                            current_label[focus_box_index].pts[(focus_point_index + 3) % 4].x() -
                            current_label[focus_box_index].pts[focus_point_index].x() +
                            current_label[focus_box_index].pts[(focus_point_index + 1) % 4].x());
                    current_label[focus_box_index].pts[(focus_point_index + 2) % 4].setY(
                            k * current_label[focus_box_index].pts[(focus_point_index + 2) % 4].x() + b);
                } else {
                    current_label[focus_box_index].pts[(focus_point_index + 2) % 4].setY(
                            current_label[focus_box_index].pts[(focus_point_index + 3) % 4].y() -
                            current_label[focus_box_index].pts[focus_point_index].y() +
                            current_label[focus_box_index].pts[(focus_point_index + 1) % 4].y());
                    current_label[focus_box_index].pts[(focus_point_index + 2) % 4].setX(
                            (current_label[focus_box_index].pts[(focus_point_index + 2) % 4].y() - b) / k);
                }
                update();
            }
            break;
        case Qt::Key_Z: // 锚点平行四边形模式
            F_mode = false;
            if (banned_point_index == -1)
                banned_point_index = focus_point_index;
            else
                banned_point_index = -1;
            break;
        case Qt::Key_X: // 定点平行四边形模式
            F_mode = false;
            if (banned_point_index == -1)
                banned_point_index = 2;
            else
                banned_point_index = -1;
            break;
        case Qt::Key_C:
            showSvg = !showSvg;  // 切换SVG显示状态
            update();  // 重绘界面
            qDebug() << "No box selected for editingeaijofnioaehdfioashoisadas";
            break;
            return;
#pragma endregion

#pragma region 多步删除当前文件
        // case Qt::Key_1: // Allow delete current file
        //     del_file = true;
        //     break;
        case Qt::Key_2: // Delete current file
            if (del_file) {
                namespace fs = std::filesystem;

                auto current_path = fs::path(current_file.toStdString());
                auto current_directory = current_path.parent_path();
                auto current_file_name = current_path.filename();
                auto trash_path = current_directory / "deleted";

                fs::is_directory(trash_path) || fs::create_directory(trash_path);
                fs::rename(current_path, trash_path / current_file_name);
                if (fs::is_regular_file(current_path.replace_extension(".txt")))
                    fs::rename(current_path.replace_extension(".txt"),
                               trash_path / current_file_name.replace_extension(".txt"));

                emit delCurrentImage();
                del_file = false;
            }
            break;
        // case Qt::Key_3: // Prohibit delete current file
        //     del_file = false;
        //     break;
#pragma endregion

#pragma region 涂黑
//        case Qt::Key_5:
//            cover_brush();
//            break;
//        case Qt::Key_4:
//            if (mode == COVER_MODE) {
//                --cover_radius;
//                update_cover(pos);
//            }
//            break;
//        case Qt::Key_6:
//            if (mode == COVER_MODE) {
//                ++cover_radius;
//                update_cover(pos);
//            }
//            break;
#pragma endregion
        default:
            break;
    }
}

void DrawOnPic::paintEvent(QPaintEvent *) {
    if (img == nullptr) return;

    // 绘制图片
    QPainter painter(this);
    painter.setTransform(img2label);
    painter.drawImage(0, 0, *img);

    // 绘制添加中的目标
    if (!adding.empty()) {
        painter.setTransform(QTransform());
        painter.setPen(pen_line);
        painter.drawPolygon(img2label.map(norm2img.map(adding)));
        painter.setPen(pen_point);
        painter.drawPoints(img2label.map(norm2img.map(adding)));
    }

    // 绘制已添加目标
    for (int i = 0; i < current_label.size(); i++) {
        const auto &box = current_label[i];
        bool is_big = label_to_size(box.tag_id, label_mode);
        // 计算svg到显示坐标系的变换
        QPolygonF painter_ploygen;
        painter_ploygen.append({0., 0.});
        painter_ploygen.append({0., (double) geometry().height()});
        painter_ploygen.append({(double) geometry().width(), (double) geometry().height()});
        painter_ploygen.append({(double) geometry().width(), 0.});
        QTransform svg2painter;
        QTransform::quadToQuad(is_big ? big_svg_ploygen : small_svg_ploygen, painter_ploygen, svg2painter);
        QPolygonF pts_on_painter = svg2painter.map(is_big ? big_pts : small_pts);
        QPolygonF pts_for_show;
        for (short i = 0; i < 4 + (label_mode == Wind); ++i) pts_for_show.append(box.pts[i]);
        // for (auto &pt: box.pts) pts_for_show.append(pt);
        pts_for_show = img2label.map(norm2img.map(pts_for_show));
        QTransform transform;
        QTransform::quadToQuad(pts_on_painter, pts_for_show, transform);
        // 绘制svg
        if (showSvg) {
            painter.setTransform(transform);
            standard_tag_render[box.tag_id].render(&painter);
        }
        // 绘制目标四边形边框
        if (i == focus_box_index) {
            painter.setPen(pen_box_focus);
        } else {
            if (box.color_id == 0) {
                pen_box.setColor(Qt::blue);
            } else if (box.color_id == 1) {
                pen_box.setColor(Qt::red);
            } else if (box.color_id == 2) {
                pen_box.setColor(Qt::green);
            } else if (box.color_id == 3) {
                pen_box.setColor(Qt::darkMagenta);
            } else {
                pen_box.setColor(Qt::green);
            }
            painter.setPen(pen_box);
        }
        painter.setTransform(QTransform());
        if(label_mode == Wind){
            QVector<QPointF> kpts;
            for (auto &pt: box.pts) kpts.append(pt);
            painter.drawPolygon(img2label.map(norm2img.map(kpts)));
        }else
            painter.drawPolygon(transform.map(painter_ploygen));
        // 绘制4个定位点
        if (i == focus_box_index) painter.setPen(pen_point_focus);
        else painter.setPen(pen_point);
        painter.drawPoints(pts_for_show);
        // 绘制标签名
        painter.setPen(pen_text);
        painter.drawText(img2label.map(norm2img.map(box.pts[0])), box.getName());
    }

    // 绘制鼠标相关
    painter.setTransform(QTransform());
    QPointF *focus = nullptr;
    switch (mode) {
        case NORMAL_MODE:
            if (draging) {
                painter.setPen(pen_line);
                painter.drawLine(QPoint(0, pos.y()), QPoint(QLabel::geometry().width(), pos.y()));
                painter.drawLine(QPoint(pos.x(), 0), QPoint(pos.x(), QLabel::geometry().height()));
                drawROI(painter);
            }
            break;
        case ADDING_MODE:
            painter.setPen(pen_line);
            painter.drawLine(QPoint(0, pos.y()), QPoint(QLabel::geometry().width(), pos.y()));
            painter.drawLine(QPoint(pos.x(), 0), QPoint(pos.x(), QLabel::geometry().height()));
            drawROI(painter);
            break;
        default:
            break;
    }
}

void DrawOnPic::setCurrentFile(QString file) {
    // 切换标注中的文件
    reset();
    current_file = file;
    loadImage();
    loadLabel();
    setNormalMode();
}

// void DrawOnPic::loadImage() {
//     // 加载图片时，需要计算两个QTransform的初值
//     // 一个用于缩放图像显示
//     // 一个用于将归一化标签坐标变为对应像素坐标
//     delete img;
//     img = new QImage();
//     img->load(current_file);
//     image_equalizeHist = false;
//     image_enhanceV = false;
//     double ratio = std::min((double) QLabel::geometry().width() / img->width(),
//                             (double) QLabel::geometry().height() / img->height());

//     QPolygonF norm_polygen;
//     norm_polygen.append({0., 0.});
//     norm_polygen.append({0., 1.});
//     norm_polygen.append({1., 1.});
//     norm_polygen.append({1., 0.});

//     QPolygonF image_polygen;
//     image_polygen.append({0., 0.});
//     image_polygen.append({0., (double) img->height()});
//     image_polygen.append({(double) img->width(), (double) img->height()});
//     image_polygen.append({(double) img->width(), 0.});

//     double x1 = (geometry().width() - img->width() * ratio) / 2;
//     double y1 = (geometry().height() - img->height() * ratio) / 2;
//     QPolygonF label_polygen;
//     label_polygen.append({x1, y1});
//     label_polygen.append({x1, y1 + ratio * img->height()});
//     label_polygen.append({x1 + ratio * img->width(), y1 + ratio * img->height()});
//     label_polygen.append({x1 + ratio * img->width(), y1});

//     QTransform::quadToQuad(norm_polygen, image_polygen, norm2img);
//     if (!stayPosition) QTransform::quadToQuad(image_polygen, label_polygen, img2label);

//     update();
// }
void DrawOnPic::loadImage() {
    delete img;
    img = new QImage();
    if (!img->load(current_file)) {
        qDebug() << "Failed to load image:" << current_file;
        return;
    }

    // 获取 DrawOnPic 控件的当前大小
    QSize labelSize = this->size();

    // 计算初始缩放比例，保持纵横比
    double scaleX = (double)labelSize.width() / img->width();
    double scaleY = (double)labelSize.height() / img->height();
    currentScale = qMin(scaleX, scaleY);

    // 计算缩放后的图片尺寸
    QSize scaledSize = img->size() * currentScale;

    // 打印信息
    // qDebug() << "DrawOnPic size:" << labelSize;
    // qDebug() << "Original image size:" << img->size();
    // qDebug() << "Scale factor:" << currentScale;
    // qDebug() << "Scaled image size:" << scaledSize;
    // qDebug() << "Window aspect ratio:" << (double)labelSize.width() / labelSize.height();
    // qDebug() << "Image aspect ratio:" << (double)img->width() / img->height();

    // 更新变换矩阵
    updateTransform();

    // 更新归一化坐标到图像坐标的变换矩阵
    QPolygonF norm_polygon({QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), QPointF(1, 0)});
    QPolygonF image_polygon({QPointF(0, 0),
                             QPointF(0, img->height()),
                             QPointF(img->width(), img->height()),
                             QPointF(img->width(), 0)});
    QTransform::quadToQuad(norm_polygon, image_polygon, norm2img);

    update();
}
void DrawOnPic::setAddingMode() {
    // 设置当前模式为正在添加一个新目标
    if (img == nullptr) return;
    mode = ADDING_MODE;
    adding.clear();
    update();
}

void DrawOnPic::setNormalMode() {
    // 设置当前模式为normal
    if (mode == ADDING_MODE)
        mode = NORMAL_MODE;
    draging = nullptr;
    adding.clear();
}

void DrawOnPic::setFocusBox(int index) {
    // 设置当前选中目标（高亮，快捷删除）
    if (0 <= index && index < current_label.size()) {
        focus_box_index = index;
        update();
    }
}
void DrawOnPic::resizeEvent(QResizeEvent *event) {
    QLabel::resizeEvent(event);
    if (img && !img->isNull()) {
        loadImage();  // 重新计算缩放和位置
    }
}

void DrawOnPic::updateTransform() {
    QSize labelSize = this->size();
    QSize scaledSize = img->size() * currentScale;

    img2label.reset();
    img2label.translate((labelSize.width() - scaledSize.width()) / 2,
                        (labelSize.height() - scaledSize.height()) / 2);
    img2label.scale(currentScale, currentScale);
}

void DrawOnPic::removeBox(QVector<box_t>::iterator box_iter) {
    int index = std::distance(current_label.begin(), box_iter);
    if (index >= 0 && index < current_label.size()) {
        current_label.erase(box_iter);
        focus_box_index = -1;
        focus_point_index = -1;
        banned_point_index = -1;
        emit labelChanged(current_label);
        update();
    }
}
// void DrawOnPic::smart() {
//     // 对当前图片进行一次智能标注。
//     if (current_file.isEmpty()) return;
//
//     lastSmartLabels = current_label;
//
//     using namespace std::chrono;
//     auto t1 = high_resolution_clock::now(); // 统计运行时间
//     if (!model.run(current_file, current_label)) {
//         // 运行失败报错
//         QMessageBox::warning(nullptr, "warning", "Cannot run smart!\n"
//                                                  "This maybe due to compiling without openvino or a broken model file.\n"
//                                                  "See warning.txt for detailed information.",
//                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
//         return;
//     }
//     auto t2 = high_resolution_clock::now(); // 统计运行时间
//     latency_ms = duration_cast<milliseconds>(t2 - t1).count();
//     qDebug("latency=%dms", latency_ms);
//     // 模型输出的像素坐标变为归一化坐标
//     for (auto &l: current_label) {
//         for (auto &pt: l.pts) {
//             pt.rx() /= img->width();
//             pt.ry() /= img->height();
//         }
//     }
//     updateBox();
// }
void DrawOnPic::setModelMode(const QString &modelMode) {
        currentModel = SmartModelFactory::createSmartModel(modelMode);
    }
 void DrawOnPic::smart() {
        if (current_file.isEmpty() || !currentModel) return;

        lastSmartLabels = current_label;

        using namespace std::chrono;
        auto t1 = high_resolution_clock::now();

        if (!currentModel->run(current_file, current_label)) {
            QMessageBox::warning(nullptr, "warning", "Model failed to run!\n"
                                                     "This may be due to compiling without proper dependencies or broken model files.\n"
                                                     "See warning.txt for detailed information.",
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            return;
        }

        auto t2 = high_resolution_clock::now();
        latency_ms = duration_cast<milliseconds>(t2 - t1).count();
        qDebug("latency=%dms", latency_ms);

        // 模型输出的像素坐标变为归一化坐标
        for (auto &l: current_label) {
            for (auto &pt: l.pts) {
                pt.rx() /= img->width();
                pt.ry() /= img->height();
            }
        }

    // 使用 TraditionalDetector 进行验证
    TraditionalDetector detector;
    cv::Mat src = cv::imread(current_file.toStdString());
    if (!src.empty()) {
        for (auto &box : current_label) {
            std::vector<cv::Point2f> points;
            for (const auto &pt : box.pts) {
                points.push_back(cv::Point2f(pt.x() * src.cols, pt.y() * src.rows));
            }

            DetectionResult result = detector.detectAndClassify(src, points);

            // 如果 TraditionalDetector 能够识别出结果，则使用其结果
            if (result.confidence > 0.5) {
                box.color_id = result.color_id;


                if (result.number == "outpost") {
                    box.tag_id = 6;
                } else if (result.number == "base") {
                    box.tag_id = result.type == Armor::Type::SMALL ? 7 : 8;
                } else if (result.number == "guard") {
                    box.tag_id = 0;
                } else {
                    int detectedNumber = -1;
                    bool isNumber = false;
                    try {
                        detectedNumber = std::stoi(result.number);
                        isNumber = true;
                    } catch (const std::exception& e) {
                        qDebug() << "Failed to convert number:" << QString::fromStdString(result.number);
                    }

                    if (isNumber) {
                        if (detectedNumber == 1 || detectedNumber == 2) {
                            box.tag_id = detectedNumber;
                        } else if (detectedNumber >= 3 && detectedNumber <= 5) {
                            if (result.type == Armor::Type::SMALL) {
                                box.tag_id = detectedNumber + 1; // 3, 4, 5 -> 4, 5, 6
                            } else {
                                box.tag_id = detectedNumber + 6; // 3, 4, 5 -> 9, 10, 11
                            }
                        } else {
                            qDebug() << "Warning: Unrecognized armor number:" << detectedNumber;
                            // 保持原有的 tag_id
                        }
                    } else {
                        qDebug() << "Warning: Invalid armor number:" << QString::fromStdString(result.number);
                        // 保持原有的 tag_id
                    }
                }

                qDebug() << "TraditionalDetector result:";
                qDebug() << "Number:" << QString::fromStdString(result.number);
                qDebug() << "Confidence:" << result.confidence;
                qDebug() << "Type:" << (result.type == Armor::Type::SMALL ? "SMALL" : "LARGE");
                qDebug() << "Color:" << QString::fromStdString(TraditionalDetector::colorToString(result.color));
                qDebug() << "Assigned tag_id:" << box.tag_id;
            } else {
                qDebug() << "TraditionalDetector couldn't confidently classify. Keeping smart() results.";
            }
        }
    } else {
        qDebug() << "Error: Unable to read image file for TraditionalDetector:" << current_file;
    }

    updateBox();
}
void DrawOnPic::updateBox() {
    update();
    emit labelChanged(current_label);
}

void DrawOnPic::reset() {
    current_file.clear();
    delete img;
    img = nullptr;
    current_label.clear();
    emit labelChanged(current_label);
    draging = nullptr;
    focus_box_index = -1;
    adding.clear();
    mode = NORMAL_MODE;
    latency_ms = -1;
}

void DrawOnPic::loadLabel() {
    // 加载当前图片对应的标签文件
    current_label.clear();
    QFileInfo image_file = current_file;
    QFileInfo label_file = image_file.absoluteFilePath().replace(image_file.suffix(), "txt");
    if (label_file.exists()) {
        QFile fp(label_file.absoluteFilePath());
        if (fp.open(QIODevice::ReadOnly)) {
            QTextStream stream(&fp);
            while (true) {
                box_t label;
                int tag;
                for(short p = 0; p < 4 + (label_mode == Wind); ++p)
                    stream >> label.pts[p].rx() >> label.pts[p].ry();
                stream >> label.tag_id >> label.color_id;
                if (stream.atEnd()) break;
                current_label.append(label);
            }
        }
    }
    emit labelChanged(current_label);
}

void DrawOnPic::saveLabel() {
    // 保存当前图片的目标到对应的标签文件
    QFileInfo image_file = current_file;
    QFileInfo label_file = image_file.absoluteFilePath().replace(image_file.suffix(), "txt");
    QFile fp(label_file.absoluteFilePath());
    if (current_label.empty()) {
        // 如果当前图片没有任何目标，则删除标签文件。
        // 主要避免之前保存过有目标的文件。
        fp.remove();
        return;
    }
    if (fp.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
        QTextStream stream(&fp);
        for (const box_t &box: current_label) {
            for (short p = 0; p < 4 + (label_mode == Wind); ++p)
                stream << box.pts[p].x() << " " << box.pts[p].y() << " ";
            stream << box.tag_id << " " << box.color_id << endl;
        }
        if (modified_img.rows)
            cv::imwrite(image_file.absoluteFilePath().toLocal8Bit().toStdString(), modified_img);
    }
}

void DrawOnPic::drawROI(QPainter &painter) {
    // 绘制ROI放大图
    QRect label_rect = QRect(QPoint(pos.x(), pos.y()) - QPoint{16, 16}, QPoint(pos.x(), pos.y()) + QPoint{16, 16});
    QRect img_rect = img2label.inverted().mapRect(label_rect);
    QImage roi_img = img->copy(img_rect).scaled(128, 128);
    painter.setTransform(QTransform());
    painter.drawImage(pos + QPoint(32, 32), roi_img);
    painter.setPen(pen_line);
    painter.drawRect(pos.x() + 32, pos.y() + 32, 128, 128);
    painter.drawLine(QPoint(pos.x() + 32 + 64, pos.y() + 32), QPoint(pos.x() + 32 + 64, pos.y() + 32 + 128));
    painter.drawLine(QPoint(pos.x() + 32, pos.y() + 32 + 64), QPoint(pos.x() + 32 + 128, pos.y() + 32 + 64));
}

// QPointF *DrawOnPic::checkPoint() {
//     // 检查当前鼠标位置距离哪个定位点最近
//     // 如果距离小于5，则判定为选中该点
//     // 用于拖动定位点
//     for (box_t &box: current_label) {
//         for (int i = 0; i < 4 + (label_mode == Wind); ++i) {
//             QPointF dif = img2label.map(norm2img.map(box.pts[i])) - pos;
//             if (dif.manhattanLength() < configure.point_distance) {
//                 return box.pts + i;
//             }
//         }
//     }
//     return nullptr;
// }
QPointF *DrawOnPic::checkPoint() {
    const double drag_radius = 20.0;  // 设置一个较大的拖动半径，单位是像素
    for (box_t &box: current_label) {
        for (int i = 0; i < 4 + (label_mode == Wind); ++i) {
            QPointF point_pos = img2label.map(norm2img.map(box.pts[i]));
            QPointF dif = point_pos - pos;
            if (QPointF::dotProduct(dif, dif) < drag_radius * drag_radius) {
                return box.pts + i;
            }
        }
    }
    return nullptr;
}
QVector<box_t> &DrawOnPic::get_current_label() {
    return current_label;
}

void DrawOnPic::stayPositionChanged(bool value) {
    stayPosition = value;
}

void DrawOnPic::illuminate() {
    if (!image_enhanceV) {
        cv::Mat channel[3];
        enh_img = modified_img.rows ? modified_img.clone() : cv::imread(current_file.toStdString());
        if (enh_img.empty()) {
            qDebug() << "Failed to load image: " << current_file;
            return;
        }
        cv::cvtColor(enh_img, enh_img, cv::COLOR_BGR2HSV);
        cv::split(enh_img, channel);
        cv::Mat x, y;
        x.Mat::create(1, 256, CV_8UC1);
        for (int i = 0; i <= 255; i++) {
            if (i * configure.V_rate > 255)
                x.at<uchar>(0, i) = 255;
            else
                x.at<uchar>(0, i) = uchar(round(configure.V_rate * i));
        }
        cv::LUT(channel[2], x, channel[2]);
        cv::merge(channel, 3, enh_img);
        cv::cvtColor(enh_img, enh_img, cv::COLOR_HSV2RGB);
        img->operator=(QImage((const unsigned char *) enh_img.data, enh_img.cols, enh_img.rows, enh_img.step,
                              QImage::Format_RGB888));
        image_enhanceV = true;
        image_equalizeHist = false;
    } else {
        image_enhanceV = false;
        if (modified_img.rows) {
            cv::cvtColor(modified_img, modified_img, cv::COLOR_RGB2BGR);
            img->operator=(QImage((const unsigned char *) modified_img.data, modified_img.cols, modified_img.rows,
                                  modified_img.step, QImage::Format_RGB888));
            cv::cvtColor(modified_img, modified_img, cv::COLOR_RGB2BGR);
        } else
            img->load(current_file);
    }
    update();
}

void DrawOnPic::histogram_Equalization() {
    if (!image_equalizeHist) {
        cv::Mat channel[4];
        enh_img = modified_img.rows ? modified_img.clone() : cv::imread(current_file.toStdString());
        if (enh_img.empty()) {
            qDebug() << "Failed to load image: " << current_file;
            return;
        }
        cv::split(enh_img, channel);
        cv::equalizeHist(channel[0], channel[0]);
        cv::equalizeHist(channel[1], channel[1]);
        cv::equalizeHist(channel[2], channel[2]);
        cv::merge(channel, 3, enh_img);
        cv::cvtColor(enh_img, enh_img, cv::COLOR_BGR2RGB);
        img->operator=(QImage((const unsigned char *) enh_img.data,
                              enh_img.cols, enh_img.rows,
                              enh_img.step, QImage::Format_RGB888));
        image_enhanceV = false;
        image_equalizeHist = true;
    } else {
        image_equalizeHist = false;
        if (modified_img.rows) {
            cv::cvtColor(modified_img, modified_img, cv::COLOR_RGB2BGR);
            img->operator=(QImage((const unsigned char *) modified_img.data,
                                  modified_img.cols, modified_img.rows,
                                  modified_img.step, QImage::Format_RGB888));
            cv::cvtColor(modified_img, modified_img, cv::COLOR_RGB2BGR);
        } else
            img->load(current_file);
    }
    update();
}

void DrawOnPic::update_cover(QPointF center) {
    center = img2label.inverted().map(pos);
    if (center.x() > 0 && center.y() > 0 && center.x() < img->width() && center.y() < img->height()) {
        cv::Mat cover_img = (image_enhanceV + image_equalizeHist ? enh_img : (modified_img.rows ? modified_img
                                                                                                : cv::imread(
                        current_file.toStdString()))).clone();
        if (!(image_enhanceV + image_equalizeHist))
            cv::cvtColor(cover_img, cover_img, cv::COLOR_RGB2BGR);
        cv::circle(cover_img, cv::Point2f(center.x(), center.y()), cover_radius, 0, -1);
        img->operator=(QImage((const unsigned char *) cover_img.data,
                              cover_img.cols, cover_img.rows,
                              cover_img.step, QImage::Format_RGB888));
    }
    update();
}

void DrawOnPic::cover_brush() {
    if (mode == NORMAL_MODE) {
        mode = COVER_MODE;
        update_cover(pos);
    } else if (mode == COVER_MODE) {
        mode = NORMAL_MODE;
        if (modified_img.rows) {
            cv::cvtColor(modified_img, modified_img, cv::COLOR_RGB2BGR);
            img->operator=(QImage((const unsigned char *) modified_img.data,
                                  modified_img.cols, modified_img.rows,
                                  modified_img.step, QImage::Format_RGB888));
            cv::cvtColor(modified_img, modified_img, cv::COLOR_RGB2BGR);
        } else
            img->load(current_file);
        if (image_enhanceV) {
            image_enhanceV = false;
            illuminate();
        } else if (image_equalizeHist) {
            image_equalizeHist = false;
            histogram_Equalization();
        }
        update();
    }
}

int DrawOnPic::label_to_size(int label, LabelMode mode)
{
    if(mode == Armor)
        return label == 1 || label > 7;
    else if(mode == Engineer)
        return label == 4;
    else
        return 0;
}

void DrawOnPic::load_svg()
{
    switch(label_mode){
        case Armor:
            standard_tag_render[0].load(QString(":/pic/tags/resource/G.svg"));
            standard_tag_render[1].load(QString(":/pic/tags/resource/1.svg"));
            standard_tag_render[2].load(QString(":/pic/tags/resource/2.svg"));
            standard_tag_render[3].load(QString(":/pic/tags/resource/3.svg"));
            standard_tag_render[4].load(QString(":/pic/tags/resource/4.svg"));
            standard_tag_render[5].load(QString(":/pic/tags/resource/5.svg"));
            standard_tag_render[6].load(QString(":/pic/tags/resource/O.svg"));
            standard_tag_render[7].load(QString(":/pic/tags/resource/Bs.svg"));
            standard_tag_render[8].load(QString(":/pic/tags/resource/Bb.svg"));
            standard_tag_render[9].load(QString(":/pic/tags/resource/B3.svg"));
            standard_tag_render[10].load(QString(":/pic/tags/resource/B4.svg"));
            standard_tag_render[11].load(QString(":/pic/tags/resource/B5.svg"));
            big_pts.clear();
            big_pts.append({0., 140.61});
            big_pts.append({0., 347.39});
            big_pts.append({871., 347.39});
            big_pts.append({871., 140.61});
            small_pts.clear();
            small_pts.append({0., 143.26});
            small_pts.append({0., 372.74});
            small_pts.append({557., 372.74});
            small_pts.append({557., 143.26});
            big_svg_ploygen.clear();
            big_svg_ploygen.append({0., 0.});
            big_svg_ploygen.append({0., 478.});
            big_svg_ploygen.append({871., 478.});
            big_svg_ploygen.append({871., 0.});
            small_svg_ploygen.clear();
            small_svg_ploygen.append({0., 0.});
            small_svg_ploygen.append({0., 516.});
            small_svg_ploygen.append({557., 516.});
            small_svg_ploygen.append({557., 0.});
            break;
        case Wind:
            standard_tag_render[0].load(QString(""));
            standard_tag_render[1].load(QString(""));
            standard_tag_render[2].load(QString(""));
            break;
        case Engineer:
            standard_tag_render[0].load(QString(":/pic/tags/resource/R.svg"));
            standard_tag_render[1].load(QString(":/pic/tags/resource/top.svg"));
            standard_tag_render[2].load(QString(":/pic/tags/resource/bottom.svg"));
            standard_tag_render[3].load(QString(":/pic/tags/resource/entrance.svg"));
            standard_tag_render[4].load(QString(":/pic/tags/resource/arrow.svg"));
            big_pts.clear();
            big_pts.append({0.5, 0.5});
            big_pts.append({86.358, 100.5});
            big_pts.append({0.5, 200.5});
            big_pts.append({100.5, 100.5});
            small_pts.clear();
            small_pts.append({0.5, 0.5});
            small_pts.append({0.5, 150.5});
            small_pts.append({150.5, 150.5});
            small_pts.append({150.5, 0.5});
            big_svg_ploygen.clear();
            big_svg_ploygen.append({0., 0.});
            big_svg_ploygen.append({0., 201.});
            big_svg_ploygen.append({101., 201.});
            big_svg_ploygen.append({101., 0.});
            small_svg_ploygen.clear();
            small_svg_ploygen.append({0., 0.});
            small_svg_ploygen.append({0., 151.});
            small_svg_ploygen.append({151., 151.});
            small_svg_ploygen.append({151., 0.});
            break;
        case Wind_Armor:
            standard_tag_render[0].load(QString(":/pic/tags/resource/anti-wind.svg"));
            small_pts.clear();
            small_pts.append({19.377, 23.514});
            small_pts.append({1.967, 226.881});
            small_pts.append({242.859, 226.628});
            small_pts.append({225.282, 22.799});
            small_svg_ploygen.clear();
            small_svg_ploygen.append({0., 0.});
            small_svg_ploygen.append({0., 250.});
            small_svg_ploygen.append({244.826, 250.});
            small_svg_ploygen.append({244.826, 0.});
            break;
    }
    emit update_list_name_signal(label_mode);
    update();
}
