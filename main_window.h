#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

#include "macros.h"
#include "qepicspv.h"

QT_BEGIN_NAMESPACE
namespace Ui { class main_window; }
QT_END_NAMESPACE

class main_window : public QMainWindow
{
    Q_OBJECT

public:
    main_window(QWidget *parent = nullptr);
    ~main_window();

private slots:
    void onWaveformReceived(QVariant& value);

    void on_btnExpert_clicked();

private:
    Ui::main_window *ui;

    QString group;
    QString prefix;
    uchar* buffer;

    QEpicsPV* waveform;
};
#endif // MAIN_WINDOW_H
