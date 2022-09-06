#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QCloseEvent>

#include "macros.h"
#include "qepicspv.h"
#include "curve_fitting.h"

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

    void closeEvent(QCloseEvent* event);

private:
    Ui::main_window *ui;

    QString group;
    QString prefix;
    uchar* buffer;
    QVector<double> xProfile;
    QVector<double> yProfile;
    QVector<double> xProfileXAxis;
    QVector<double> yProfileXAxis;
    QVector<double> xProfileFit;
    QVector<double> yProfileFit;
    int sum;
    int rawValue;

    QEpicsPV* waveform;
};
#endif // MAIN_WINDOW_H
