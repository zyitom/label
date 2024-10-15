#include "labeldialog.h"
#include "ui_labeldialog.h"
#include <iostream>
#include <QObject>
#include <QDebug>
#include <QShortcut>
#include <QApplication>
#include <QDialog>
#include <QTimer>
#include <QApplication>
#include <QTimer>

int last_tag_id = 0, last_color_id = 0;
static const QString Armor_names[] = {"Sentry", "Hero", "Engineer", "3-Infantry", "4-Infantry", "5-Infantry", "Outpost", "Base-small", "Base-big", "3-Balance", "4-Balance", "5-Balance"};
static const QString Engineer_names[] = {"R-tag", "Top", "Bottom", "Entrance", "Arrow"};
static const QString Wind_names[] = {"Dark", "To-hit", "Hit"};
static const QString Color_names[] = {"Blue", "Red", "Extinguish", "Purple"};

LabelDialog::LabelDialog(QVector<box_t>::iterator box_iter, const LabelMode mode, QWidget *parent)
    : QDialog(parent), ui(new Ui::LabelDialog), current_box(box_iter) {
    ui->setupUi(this);
    setup_boxes(mode);
    ui->class_box->setCurrentIndex(current_box->tag_id);
    ui->color_box->setCurrentIndex(current_box->color_id);
    
    // 设置删除按钮为默认按钮并给予焦点
    ui->deleteButton->setDefault(true);
    ui->deleteButton->setFocus();

    QWidget::connect(ui->cancelButton, &QPushButton::pressed, this, &QDialog::hide);
    
    // 为回车键添加快捷方式
    QShortcut *enterShortcut = new QShortcut(QKeySequence(Qt::Key_Return), this);
    connect(enterShortcut, &QShortcut::activated, this, &LabelDialog::handleEnterKey);

    // 连接 color_box 和 class_box 的 currentIndexChanged 信号
    connect(ui->color_box, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LabelDialog::onComboBoxChanged);
    connect(ui->class_box, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LabelDialog::onComboBoxChanged);

    
    this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);
    this->installEventFilter(this);}



void LabelDialog::activateAndFocus() {
    this->show();
    this->raise();
    this->activateWindow();
    ui->saveButton->setFocus();
}

bool LabelDialog::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::WindowDeactivate) {
        QTimer::singleShot(0, this, &LabelDialog::ensureDialogFocus);
    }
    return QDialog::eventFilter(obj, event);
}

void LabelDialog::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    QTimer::singleShot(0, this, &LabelDialog::ensureDialogFocus);
}

void LabelDialog::changeEvent(QEvent *event) {
    QDialog::changeEvent(event);
    if (event->type() == QEvent::ActivationChange && this->isActiveWindow()) {
        ui->saveButton->setFocus();
    }
}

void LabelDialog::ensureDialogFocus() {
    if (!this->isActiveWindow()) {
        this->activateWindow();
        this->raise();
        ui->saveButton->setFocus();
    }
}

void LabelDialog::onComboBoxChanged(int) {
    ui->saveButton->setFocus();
    ui->saveButton->setDefault(true);
    ui->deleteButton->setDefault(false);
    QTimer::singleShot(0, this, &LabelDialog::ensureDialogFocus);
}

void LabelDialog::handleEnterKey() {
    if (ui->deleteButton->hasFocus()) {
        on_deleteButton_pressed();
    } else {
        on_saveButton_pressed();
    }
}


void LabelDialog::on_saveButton_pressed() {
    current_box->set_class(ui->color_box->currentIndex(), ui->class_box->currentIndex());
    last_tag_id = current_box->tag_id;
    last_color_id = current_box->color_id;
    emit changeBoxEvent();
    this->accept();
}

void LabelDialog::on_deleteButton_pressed() {
    emit removeBoxEvent(current_box);
    this->accept();
}

LabelDialog::~LabelDialog() {
    delete ui;
}
void LabelDialog::closeEvent(QCloseEvent *event) {
    QDialog::closeEvent(event);
    QWidget *parent = parentWidget();
    if (parent) {
        parent->activateWindow();
        parent->setFocus();
    }
}
void update_list_name(const LabelMode mode){
    switch(mode){
        case Armor:
            for (short i = 0; i < 12; ++i) tag_name[i] = Armor_names[i];
            break;
        case Wind:
            for (short i = 0; i < 3; ++i) tag_name[i] = Wind_names[i];
            break;
        case Wind_Armor:
            tag_name[0] = "Wind";
            break;
        case Engineer:
            for (short i = 0; i < 5; ++i) tag_name[i] = Engineer_names[i];
            break;
    }
}

void LabelDialog::setup_boxes(const LabelMode mode)
{
    ui->color_box->clear();
    ui->class_box->clear();
    switch(mode){
        case Armor:
            for (auto &name:Armor_names) ui->class_box->addItem(name);
            for (auto &name:Color_names) ui->color_box->addItem(name);
            break;
        case Wind:
            for (auto &name:Wind_names) ui->class_box->addItem(name);
            ui->color_box->addItem(Color_names[0]);
            ui->color_box->addItem(Color_names[1]);
            break;
        case Wind_Armor:
            ui->class_box->addItem("Wind");
            ui->color_box->addItem(Color_names[0]);
            ui->color_box->addItem(Color_names[1]);
            break;
        case Engineer:
            for (auto &name:Engineer_names) ui->class_box->addItem(name);
            ui->color_box->addItem("Gold");
            break;
    }
}
