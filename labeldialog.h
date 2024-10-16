#ifndef LABELDIALOG_H
#define LABELDIALOG_H

#include <QDialog>
#include "drawonpic.h"

extern int last_tag_id, last_color;

namespace Ui {
    class LabelDialog;
}

class LabelDialog : public QDialog {
Q_OBJECT

public:
    explicit LabelDialog(QVector<box_t>::iterator box_iter, const LabelMode mode, QWidget *parent = 0);

    ~LabelDialog();

    void setup_boxes(const LabelMode mode);

    void activateAndFocus();

signals:

    void removeBoxEvent(QVector<box_t>::iterator box_iter);

    void changeBoxEvent();

private slots:

    void on_saveButton_pressed();
    void on_deleteButton_pressed();
    void handleEnterKey();
    void onComboBoxChanged(int);
protected:
 bool eventFilter(QObject *obj, QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    Ui::LabelDialog *ui = nullptr;

    QVector<box_t>::iterator current_box;

    void ensureDialogFocus();
protected:
    void closeEvent(QCloseEvent *event) override;
};

void update_list_name(const LabelMode mode);

#endif // LABELDIALOG_H
