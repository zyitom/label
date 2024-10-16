#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QFileDialog>
#include <QCoreApplication>
#include <QMessageBox>
#include <QTimer>
#include <drawonpic.h>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include <QDir>

class IndexQListWidgetItem : public QListWidgetItem {
public:
    IndexQListWidgetItem(QString name, int index) : QListWidgetItem(name), index(index) {

    }

    int getIndex() const { return index; }
    void setIndex(const int value) { this->index = value; }

private:
    int index;
};
MainWindow::MainWindow(QWidget *parent, std::string path, int init_mode) :
        QMainWindow(parent),
        ui(new Ui::MainWindow) {
    ui->setupUi(this);
    cdialog = new Configuredialog(&ui->label->configure, this);
    QObject::connect(ui->addLabelPushButton, &QPushButton::clicked, ui->label, &DrawOnPic::setAddingMode);
    QObject::connect(ui->savePushButton, &QPushButton::clicked, ui->label, &DrawOnPic::saveLabel);
    QObject::connect(ui->stayPositionCheckBox, &QCheckBox::toggled, ui->label, &DrawOnPic::stayPositionChanged);
    QObject::connect(ui->illuminateButton, &QPushButton::clicked, ui->label, &DrawOnPic::illuminate);
    QObject::connect(ui->histogramEqualizationButton, &QPushButton::clicked, ui->label, &DrawOnPic::histogram_Equalization);
    QObject::connect(ui->coverBrushButton, &QPushButton::clicked, ui->label, &DrawOnPic::cover_brush);
    QObject::connect(ui->label, &DrawOnPic::update_list_name_signal, [=](){update_list_name(ui->label->label_mode);});
    QObject::connect(ui->configurePushButton, &QPushButton::clicked, [=](){cdialog->show_configure();});
    QObject::connect(ui->label, &DrawOnPic::labelDialogClosed, this, &MainWindow::onLabelDialogClosed);
    QObject::connect(ui->savePushButton, &QPushButton::clicked, this, &MainWindow::on_savePushButton_clicked);
    QObject::connect(ui->label, &DrawOnPic::delCurrentImage, [=]() {
        ui->label->enh_img = NULL_IMG;
        ui->label->modified_img = NULL_IMG;
        ui->label->image_enhanceV = ui->label->image_equalizeHist = false;
        auto currentRow = ui->fileListWidget->currentRow();
        auto count =  ui->fileListWidget->count();
        for (int i = currentRow + 1; i < count; i++) {
            auto current = static_cast<IndexQListWidgetItem *>(ui->fileListWidget->item(i));
            current->setIndex(current->getIndex()-1);
        }
        ui->fileListWidget->removeItemWidget(ui->fileListWidget->takeItem(currentRow));
        ui->fileListHorizontalSlider->setMaximum(ui->fileListWidget->count());
    });
    ui->label->label_mode = LabelMode(ui->label->configure.last_mode);
    labelModelName = ui->labelModelName;
    ui->autoSaveCheckBox->setCheckState(Qt::Checked);  
    ui->modeComboBox->setCurrentIndex(ui->label->configure.last_mode);
    ui->modelTypeComboBox->setCurrentIndex(ui->label->configure.last_model_type);
    ui->autoEnhanceVCheckBox->setCheckState(ui->label->configure.auto_enhance_V ? Qt::Checked : Qt::Unchecked);
    update_list_name(ui->label->label_mode);
    initializeModel(ui->label->configure.last_model_type);
 bool last_flag = false;
    if(path == ""){
        path = ui->label->configure.last_open.toStdString();
        last_flag = true;
    }
    if(path != "" && path != "."){
        std::string tfile = "";
        int file_index = 1;
        if (QFileInfo(QString::fromStdString(path)).isFile()) {
            tfile = path.substr(path.find_last_of('/') + 1);
            path = path.substr(0, path.find_last_of('/'));
        }
        
        // Use the loadFiles function here
        loadFiles(QString::fromStdString(path));
        
        // Set the current item and slider value
       if (!tfile.empty()) {
    for (int i = 0; i < ui->fileListWidget->count(); ++i) {
        QListWidgetItem* item = ui->fileListWidget->item(i);
        if (QFileInfo(item->data(Qt::UserRole).toString()).fileName() == QString::fromStdString(tfile)) {
            ui->fileListWidget->setCurrentRow(i);
            file_index = i + 1;
            break;
        }
    }
        } else {
            ui->fileListWidget->setCurrentRow(0);
        }
        
        ui->fileListHorizontalSlider->setValue(last_flag ? ui->label->configure.last_pic : file_index);
        ui->label->setFocusPolicy(Qt::StrongFocus);
        this->installEventFilter(this);
    }
}

void MainWindow::onLabelDialogClosed(int boxIndex)
{
    // 确保 labelListWidget 存在且有足够的项
    if (ui->labelListWidget && ui->labelListWidget->count() > boxIndex) {
        ui->labelListWidget->setCurrentRow(boxIndex);
        ui->labelListWidget->setFocus();
    }
}

void MainWindow::initializeModel(int modelType) {
    QString modelMode;
    switch (modelType) {
        case 0:
            modelMode = "YOLOX";
            break;
        case 1:
            modelMode = "YOLOv5";
            break;
    }

    ui->label->setModelMode(modelMode);
    qDebug() << "Model initialized to:" << modelMode;
}

MainWindow::~MainWindow() {
    ui->label->configure.last_pic = ui->fileListHorizontalSlider->value();
    delete cdialog;
    delete ui;
}
QString MainWindow::getModelFileName(const QString& fullPath) {
    return QFileInfo(fullPath).fileName();
}

// void MainWindow::on_openDirectoryPushButton_clicked() {
//     ui->label->reset();     // 重置绘图控件
//     QStringList image_filter = {"*.jpg", "*.png", "*.jpeg"};    // 支持的图像格式
//     QDir dir;
//     dir = QFileDialog::getExistingDirectory(this, "", ui->label->configure.last_open, QFileDialog::ShowDirsOnly);
//     ui->fileListWidget->clear();    // 清空文件列表
//     int idx = 0;
//     // 遍历文件夹下的图片，并添加到文件列表
//     for (QString file: dir.entryList(image_filter)) {
//         if (file == "." || file == "..") continue;
//         ui->fileListWidget->addItem(new IndexQListWidgetItem(dir.absoluteFilePath(file), idx++));
//     }
//     ui->fileListWidget->setCurrentItem(ui->fileListWidget->item(0));
//     // 设置拖动条
//     ui->fileListHorizontalSlider->setMinimum(1);
//     ui->fileListHorizontalSlider->setMaximum(ui->fileListWidget->count());
//     ui->fileListHorizontalSlider->setValue(1);
//     ui->label->configure.last_open = dir.path();
// }

// void MainWindow::on_fileListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
//     if (current == nullptr) return;
//     // 修改绘图区的内容
//     ui->label->setCurrentFile(current->text());
//     // 更新拖动条位置
//     int idx = static_cast<IndexQListWidgetItem *>(current)->getIndex();
//     ui->fileListHorizontalSlider->setValue(idx + 1);
// }

void MainWindow::on_label_labelChanged(const QVector<box_t> &labels) {
    // 当添加/删除/修改当前图片的label时，重置右上角的标签列表
    // TODO: 可以有性能优化
    IndexQListWidgetItem *tilwi;
    static const QColor color_list[] = {Qt::blue, Qt::red, Qt::green, Qt::darkMagenta};
    ui->labelListWidget->clear();
    for (int i = 0; i < labels.size(); i++) {
        tilwi = new IndexQListWidgetItem(labels[i].getName(), i);
        tilwi->setForeground(color_list[labels[i].color_id]);
        ui->labelListWidget->addItem(tilwi);
    }
}

void MainWindow::on_labelListWidget_itemDoubleClicked(QListWidgetItem *item) {
    int idx = static_cast<IndexQListWidgetItem *>(item)->getIndex();
    lastEditedLabelIndex = idx; // 记录当前编辑的标签索引
    delete dialog;
    dialog = new LabelDialog(ui->label->get_current_label().begin() + idx, ui->label->label_mode);
    dialog->setModal(true);
    QObject::connect(dialog, &LabelDialog::removeBoxEvent, ui->label, &DrawOnPic::removeBox);
    QObject::connect(dialog, &LabelDialog::changeBoxEvent, ui->label, &DrawOnPic::updateBox);
    QObject::connect(dialog, &QDialog::finished, this, &MainWindow::onLabelDialogFinished);
    dialog->show();
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_D) {
            qDebug() << "D key pressed in MainWindow";
            ui->label->openLabelDialog();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}


// void MainWindow::on_modelTypeComboBox_currentIndexChanged(int index) {
//     switch (index) {
//         case 0:
//             ui->label->configure.last_model_name = "model-opt.onnx";
//         break;
//         case 1:
//             ui->label->configure.last_model_name = "another-model.onnx";
//         break;
//         // Add more model names as needed
//         // default:
//         //     ui->label->configure.last_model_name = "default-model.onnx";
//         // break;
//     }
//     ui->label->configure.model_path = "../resources/" + ui->label->configure.last_model_name;
//     ui->label->configure.last_model_type = index;
//     qDebug() << "Model path changed to:" << ui->label->configure.model_path;

//     // Update the model name label
//     labelModelName->setText(getModelFileName(ui->label->configure.last_model_name));
// }

// void MainWindow::on_selectModelPushButton_clicked() {
//     static int callCount = 0;
//     qDebug() << "Function called " << ++callCount << " times";
//     QString fileName = QFileDialog::getOpenFileName(this,
//         tr("Select ONNX Model"), "",
//         tr("ONNX Models (*.onnx);;All Files (*)"));
//     if (!fileName.isEmpty()) {
//         qDebug() << "Selected ONNX model:" << fileName;
//         ui->label->configure.model_path = fileName;
//         ui->label->configure.last_model_name = QFileInfo(fileName).fileName();

//         // Update the model name label
//         labelModelName->setText(ui->label->configure.last_model_name);
//     }
// }
void MainWindow::onLabelDialogFinished(int result) {
    if (result == QDialog::Accepted) {
        QTimer::singleShot(0, this, [this]() {
            if (lastEditedLabelIndex < ui->labelListWidget->count()) {
                ui->labelListWidget->setCurrentRow(lastEditedLabelIndex);
                ui->labelListWidget->setFocus();
                ui->label->updateBox(); // 立即更新绘图区域
                on_label_labelChanged(ui->label->get_current_label()); // 更新右侧标签列表
            }
            this->activateWindow();
            this->setFocus();
        });
    } else {
        this->activateWindow();
        this->setFocus();
    }
}

void MainWindow::on_labelListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    // 更新当前选中目标（绘图区域目标高亮）
    if (current == nullptr) return;
    int idx = static_cast<IndexQListWidgetItem *>(current)->getIndex();
    ui->label->setFocusBox(idx);
}

// void MainWindow::on_smartPushButton_clicked() {
//     // 进行一次自动标注
//     ui->label->smart();
// }
void MainWindow::on_modelTypeComboBox_currentIndexChanged(int index) {
    initializeModel(index);
    ui->label->configure.last_model_type = index;
}

    void MainWindow::on_smartPushButton_clicked() {
        // 进行一次自动标注
        ui->label->smart();
    }
/*
void MainWindow::on_smartAllPushButton_clicked() {
    // 遍历所有目标，依次进行自动标注
    // TODO: 目前无法中断该过程
    for (int i = 0; i < ui->fileListWidget->count(); i++) {
        ui->fileListWidget->setCurrentRow(i);
        ui->label->smart();
        // ui->label->setCurrentFile(ui->fileListWidget->item(i)->text());
        ui->label->saveLabel();
        QCoreApplication::processEvents();
    }
}
*/

void MainWindow::on_nextPushButton_clicked() {
    // 切换图片时，判断是否需要自动保存
    if (ui->autoSaveCheckBox->checkState() == Qt::Checked) {
        ui->label->saveLabel();
    }
    // 更新左下角文件列表中的选中项（通过信号更新绘图区域内容）
    int next_idx = ui->fileListWidget->currentRow() + 1;
    if (next_idx < ui->fileListWidget->count()) {
        ui->fileListWidget->setCurrentRow(next_idx);
    }
}

// 切换为上一个目标。和切换下一个目标类似。
void MainWindow::on_prevPushButton_clicked() {
    if (ui->autoSaveCheckBox->checkState() == Qt::Checked) {
        ui->label->saveLabel();
    }
    int prev_idx = ui->fileListWidget->currentRow() - 1;
    if (prev_idx >= 0) {
        ui->fileListWidget->setCurrentRow(prev_idx);
    }
}

void MainWindow::on_fileListHorizontalSlider_valueChanged(int value) {
    // 响应文件列表拖动条
    ui->fileListLabel->setText(QString::asprintf("[%d/%d]", value, ui->fileListHorizontalSlider->maximum()));
    ui->fileListWidget->setCurrentItem(ui->fileListWidget->item(value - 1));
    if(ui->label->configure.auto_enhance_V)
        ui->label->illuminate();
    else{
        ui->label->enh_img = NULL_IMG;
        ui->label->modified_img = NULL_IMG;
        ui->label->image_enhanceV = ui->label->image_equalizeHist = false;
    }
}

void MainWindow::on_fileListHorizontalSlider_rangeChanged(int min, int max) {
    // 响应文件列表拖动条
    ui->fileListLabel->setText(QString::asprintf("[%d/%d]", ui->fileListHorizontalSlider->value(), max));
}

void MainWindow::on_modeComboBox_currentIndexChanged(int i){
    ui->label->label_mode = LabelMode(i);
    //qDebug() <<i;
    ui->label->load_svg();
    ui->label->configure.last_mode = i;
    on_label_labelChanged(ui->label->get_current_label()); // 立即更新右侧标签列表
    ui->label->update(); // 更新绘图区域
}

void MainWindow::on_interpolateButton_clicked() {
    enum InterpolateStatus {
        Idle, Waiting4A, Waiting4B
    };// 三种状态：闲置，准备选择第一个插值点，准备选择第二个插值点
    static InterpolateStatus status = Idle;
    static int target_idx = 0;
    static box_t box_a, box_b;
    switch (status) {
        case Idle:
            target_idx = ui->fileListWidget->currentRow();
//            if (target_idx - 1 < 0 || target_idx + 1 >= ui->fileListWidget->count()) { //考虑到后面实际上可以随意选择插值来源，这个判断可有可无
//                QMessageBox::warning(this, "Invalid interpolate", "Can't interpolate boxes on first or last picture.");
//            } else {
            if (ui->autoSaveCheckBox->isChecked()) ui->label->saveLabel();
            ui->fileListWidget->setCurrentRow(target_idx - 1); // 自动跳转到上一张图
            status = Waiting4A;
//            }
            break;
        case Waiting4A:
        case Waiting4B: {
            auto selected = ui->labelListWidget->selectedItems();
            if (selected.empty()) {
                QMessageBox::warning(this, "No boxes selected", "Select a box first");
            } else {
                if (status == Waiting4A) { // 读取到box_a
                    box_a = ui->label->get_current_label().at(
                            (dynamic_cast<IndexQListWidgetItem *>(selected.first()))->getIndex());
                    ui->fileListWidget->setCurrentRow(target_idx + 1); // 自动跳转到下一张
                    status = Waiting4B;
                } else {
                    box_b = ui->label->get_current_label().at(
                            (dynamic_cast<IndexQListWidgetItem *>(selected.first()))->getIndex());
                    if (box_a.tag_id == box_b.tag_id) { // 检测装甲板类型是否相同
                        box_t result;
                        result.tag_id = box_a.tag_id;
                        result.color_id = box_a.color_id;
                        for (int i = 0; i < 4; ++i) result.pts[i] = box_a.pts[i] / 2 + box_b.pts[i] / 2; // 插值
                        ui->fileListWidget->setCurrentRow(target_idx);
                        ui->label->get_current_label().append(result);
                        ui->label->updateBox();
                    } else {
                        QMessageBox::warning(this, "Shouldn't interpolate boxes of different types",
                                             "Please select boxes of same type");
                        ui->fileListWidget->setCurrentRow(target_idx);
                    }
                    status = Idle;
                }
            }
        }
            break;
    }
}
void MainWindow::on_upLabelButton_clicked() {
    int currentRow = ui->labelListWidget->currentRow();
    int totalLabels = ui->labelListWidget->count();

    if (totalLabels == 0) {
        qDebug() << "No labels available";
        return;
    }

    // 计算新的索引，向上循环
    int newIdx = (currentRow - 1 + totalLabels) % totalLabels;
    ui->labelListWidget->setCurrentRow(newIdx);

    if (newIdx >= 0 && newIdx < ui->label->get_current_label().size()) {
        const box_t& current_box = ui->label->get_current_label()[newIdx];

        // 打印坐标
        qDebug() << "Current label coordinates:";
        for (int i = 0; i < 4; ++i) {
            qDebug() << "Point" << i + 1 << ": ("
                     << current_box.pts[i].x() << ", "
                     << current_box.pts[i].y() << ")";
        }

        // 计算标签的边界和中心点
        qreal minX = 1.0, minY = 1.0, maxX = 0.0, maxY = 0.0;
        QPointF center(0.0, 0.0);
        for (int i = 0; i < 4; ++i) {
            minX = qMin(minX, current_box.pts[i].x());
            minY = qMin(minY, current_box.pts[i].y());
            maxX = qMax(maxX, current_box.pts[i].x());
            maxY = qMax(maxY, current_box.pts[i].y());
            center += current_box.pts[i];
        }
        center /= 4.0;

        // 计算目标区域的宽高
        qreal targetWidth = maxX - minX;
        qreal targetHeight = maxY - minY;

        // 添加边距
        qreal padding = 0.5; // 50%的边距
        targetWidth *= (1 + padding);
        targetHeight *= (1 + padding);

        // 获取当前的变换矩阵
        QTransform currentTransform = ui->label->img2label;

        // 计算需要的缩放比例
        QRectF currentRect = currentTransform.mapRect(QRectF(0, 0, ui->label->img->width(), ui->label->img->height()));
        qreal currentScale = qMin(currentRect.width() / ui->label->img->width(), currentRect.height() / ui->label->img->height());

        qreal desiredScale = qMin(ui->label->width() / (targetWidth * ui->label->img->width()),
                                  ui->label->height() / (targetHeight * ui->label->img->height()));

        qreal scaleFactor = desiredScale / currentScale;

        // 限制最大缩放比例变化
        scaleFactor = qBound(0.5, scaleFactor, 2.0);

        // 计算新的变换矩阵
        QTransform newTransform;
        newTransform.translate(ui->label->width() / 2, ui->label->height() / 2);
        newTransform.scale(desiredScale, desiredScale);
        newTransform.translate(-center.x() * ui->label->img->width(), -center.y() * ui->label->img->height());

        // 应用新的变换
        ui->label->img2label = newTransform;
        ui->label->update();

        qDebug() << "Applied scale factor:" << scaleFactor;
        qDebug() << "Center point:" << center;
    } else {
        qDebug() << "Invalid label index";
    }
}
void MainWindow::on_delImageButton_clicked()
{
    QListWidgetItem* currentItem = ui->fileListWidget->currentItem();
    if (!currentItem) return;

    int currentRow = ui->fileListWidget->currentRow();
    QString imageFile = currentItem->data(Qt::UserRole).toString();
    QString textFile = imageFile;
    textFile.replace(QRegExp("\\.(png|jpg|jpeg)$"), ".txt");

    qDebug() << "Attempting to delete image file:" << imageFile;
    qDebug() << "Corresponding text file:" << textFile;

    QProcess process;
    QStringList args;

    // Check if image file exists
    if (!QFile::exists(imageFile)) {
        qDebug() << "Image file does not exist:" << imageFile;
        QMessageBox::warning(this, "Error", "Image file does not exist: " + imageFile);
        return;
    }

    // Move image file to trash
    args << "trash" << imageFile;
    process.start("gio", args);
    process.waitForFinished();

    if (process.exitCode() == 0) {
        qDebug() << "Image file moved to trash successfully";

        // Check if text file exists and move to trash if it does
        if (QFile::exists(textFile)) {
            args.clear();
            args << "trash" << textFile;
            process.start("gio", args);
            process.waitForFinished();

            if (process.exitCode() == 0) {
                qDebug() << "Text file moved to trash successfully";
            } else {
                qDebug() << "Failed to move text file to trash. Error:" << process.errorString();
            }
        }

        // Remove the item from the list
        delete ui->fileListWidget->takeItem(currentRow);

        // Update indices of remaining items
        for (int i = currentRow; i < ui->fileListWidget->count(); ++i) {
            IndexQListWidgetItem* item = static_cast<IndexQListWidgetItem*>(ui->fileListWidget->item(i));
            item->setIndex(item->getIndex() - 1);
        }

        // Update slider maximum
        ui->fileListHorizontalSlider->setMaximum(ui->fileListWidget->count());

        // Move to next image (or last if at the end)
        if (ui->fileListWidget->count() > 0) {
            if (currentRow >= ui->fileListWidget->count()) {
                currentRow = ui->fileListWidget->count() - 1;
            }
            ui->fileListWidget->setCurrentRow(currentRow);
        } else {
            // Clear label and reset UI if no more images
            ui->label->clear();
            ui->fileListLabel->setText("[0/0]");
            ui->fileListHorizontalSlider->setValue(0);
        }

        // This will trigger on_fileListWidget_currentItemChanged
        ui->fileListWidget->setCurrentRow(currentRow);
    } else {
        qDebug() << "Failed to move image file to trash. Error:" << process.errorString();
        QMessageBox::warning(this, "Error", "Failed to move image file to trash. Error: " + process.errorString());
    }
}
void MainWindow::updateFileListItemColor(QListWidgetItem* item) {
    if (!item) return;
    QString fullPath = item->data(Qt::UserRole).toString();
    QString textFile = fullPath;
    textFile.replace(QRegExp("\\.(png|jpg|jpeg)$"), ".txt");

    if (QFile::exists(textFile)) {
        item->setForeground(Qt::blue);
    } else {
        item->setForeground(Qt::black);
    }
}


void MainWindow::loadFiles(const QString& path) {
    QDir dir(path);
    QStringList image_filter = {"*.jpg", "*.png", "*.jpeg"};
    ui->fileListWidget->clear();
    int idx = 0;
    for (const QString& file : dir.entryList(image_filter, QDir::Files)) {
        QString fullPath = dir.absoluteFilePath(file);
        auto item = new IndexQListWidgetItem(QFileInfo(file).fileName(), idx++);
        item->setData(Qt::UserRole, fullPath);  // 存储完整路径作为用户数据
        ui->fileListWidget->addItem(item);
        updateFileListItemColor(item);
    }
    ui->fileListHorizontalSlider->setMinimum(1);
    ui->fileListHorizontalSlider->setMaximum(ui->fileListWidget->count());
    if (ui->fileListWidget->count() > 0) {
        ui->fileListWidget->setCurrentRow(0);
    }
}

void MainWindow::on_openDirectoryPushButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "", ui->label->configure.last_open, QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty()) {
        loadFiles(dir);
        ui->label->configure.last_open = dir;
    }
}
void MainWindow::on_savePushButton_clicked() {
    ui->label->saveLabel();
    QListWidgetItem* currentItem = ui->fileListWidget->currentItem();
    if (currentItem) {
        updateFileListItemColor(currentItem);
        currentItem->setData(Qt::UserRole + 1, true);  // 更新缓存的标注状态
    }
}


void MainWindow::on_fileListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (current == nullptr) return;

    // Update previous item's color (if exists)
    if (previous) {
        updateFileListItemColor(previous);
    }

    // Handle current item
    QString fullPath = current->data(Qt::UserRole).toString();
    ui->label->setCurrentFile(fullPath);
    int idx = static_cast<IndexQListWidgetItem *>(current)->getIndex();
    ui->fileListHorizontalSlider->setValue(idx + 1);

    // Update current item's color
    updateFileListItemColor(current);

    // Update file list label
    ui->fileListLabel->setText(QString::asprintf("[%d/%d]", idx + 1, ui->fileListWidget->count()));

    // Refresh label view
    ui->label->update();

    // If auto-enhance is enabled, apply enhancement
    if (ui->label->configure.auto_enhance_V) {
        ui->label->illuminate();
    } else {
        ui->label->enh_img = NULL_IMG;
        ui->label->modified_img = NULL_IMG;
        ui->label->image_enhanceV = ui->label->image_equalizeHist = false;
    }
}

void MainWindow::updateUIAfterDeletion()
{
    // 从文件列表中移除项目
    int currentRow = ui->fileListWidget->currentRow();
    QListWidgetItem* item = ui->fileListWidget->takeItem(currentRow);
    if (item) {
        delete item;
        qDebug() << "Item removed from fileListWidget at row:" << currentRow;
    } else {
        qDebug() << "Failed to remove item from fileListWidget at row:" << currentRow;
    }

    // 更新剩余项目的索引
    for (int i = currentRow; i < ui->fileListWidget->count(); ++i) {
        IndexQListWidgetItem* indexItem = dynamic_cast<IndexQListWidgetItem*>(ui->fileListWidget->item(i));
        if (indexItem) {
            indexItem->setIndex(indexItem->getIndex() - 1);
            qDebug() << "Updated index for item at row" << i << "to" << indexItem->getIndex();
        }
    }

    // 更新滑动条最大值
    ui->fileListHorizontalSlider->setMaximum(ui->fileListWidget->count());
    qDebug() << "Updated fileListHorizontalSlider maximum to:" << ui->fileListWidget->count();

    // 移动到下一个图像（如果有的话）
    if (ui->fileListWidget->count() > 0) {
        if (currentRow >= ui->fileListWidget->count()) {
            currentRow = ui->fileListWidget->count() - 1;
        }
        ui->fileListWidget->setCurrentRow(currentRow);
        qDebug() << "Set current row to:" << currentRow;

        // 更新文件列表标签
        ui->fileListLabel->setText(QString::asprintf("[%d/%d]", currentRow + 1, ui->fileListWidget->count()));
        qDebug() << "Updated fileListLabel to:" << ui->fileListLabel->text();

        // 更新滑动条值
        ui->fileListHorizontalSlider->setValue(currentRow + 1);
        qDebug() << "Set fileListHorizontalSlider value to:" << (currentRow + 1);

        // 加载新的当前文件
        QListWidgetItem* currentItem = ui->fileListWidget->currentItem();
        if (currentItem) {
            QString newFile = currentItem->text();
            ui->label->setCurrentFile(newFile);
            qDebug() << "Loaded new current file:" << newFile;
        }
    } else {
        // 如果没有更多图像，清除标签并重置UI
        ui->label->clear();
        ui->fileListLabel->setText("[0/0]");
        ui->fileListHorizontalSlider->setValue(0);
        qDebug() << "No more images. Cleared label and reset UI";
    }

    // 刷新标签视图
    ui->label->update();
    qDebug() << "Label view refreshed";
}
void MainWindow::on_downLabelButton_clicked() {
    int next_idx = ui->labelListWidget->currentRow() + 1;
    if (next_idx < ui->labelListWidget->count()) {
        ui->labelListWidget->setCurrentRow(next_idx);
    } else {
        ui->labelListWidget->setCurrentRow(0);  // 循环到第一个标签
    }
}

void MainWindow::on_autoEnhanceVCheckBox_stateChanged(int check) {
    if(check != 1){
        ui->label->configure.auto_enhance_V = check > 0;
        if(check == 2 && !ui->label->image_enhanceV || check == 0 && ui->label->image_enhanceV)
            ui->label->illuminate();
    }
}
