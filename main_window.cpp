#include "main_window.h"
#include "ui_main_window.h"

#include <iostream>
#include <algorithm>

using namespace std;

main_window::main_window(QString prefix, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::main_window)
{
    ui->setupUi(this);
    this->prefix = prefix;
    this->group = this->prefix + ":CAM";
    setWindowTitle(this->prefix);

    SET_GROUP(QELabel);
    SET_GROUP(QENumericEdit);
    SET_GROUP(QEComboBox);
    SET_GROUP(QEPushButton);

    this->waveform = new QEpicsPV(this->prefix + ":image1:ArrayData");
    QObject::connect(this->waveform, SIGNAL(valueUpdated(QVariant&)), this, SLOT(onWaveformReceived(QVariant&)));

    sum = 0;
    rawValue = 0;

    ui->plotProfileX->addGraph();
    ui->plotProfileX->addGraph();
    ui->plotProfileX->graph(1)->setPen(QPen(QColor(Qt::red)));
    ui->plotProfileX->graph(1)->setValueAxis(ui->plotProfileX->yAxis2);

    ui->plotProfileY->addGraph();
    ui->plotProfileY->addGraph();
    ui->plotProfileY->graph(1)->setPen(QPen(QColor(Qt::red)));
    ui->plotProfileY->graph(1)->setValueAxis(ui->plotProfileY->yAxis2);

    QObject::connect(this, SIGNAL(close()), this->ui->btnStop, SLOT(clicked()));
}

main_window::~main_window()
{
    delete ui;
}

void main_window::onWaveformReceived(QVariant& value)
{
    QStringList rawData = value.toStringList();
    int length = ui->lblSizeX->text().toInt() * ui->lblSizeY->text().toInt();
    this->buffer = new uchar[length];
    this->xProfile = QVector<double>( ui->lblSizeY->text().toInt() );
    this->yProfile = QVector<double>( ui->lblSizeX->text().toInt() );
    this->xProfileXAxis = QVector<double>( ui->lblSizeY->text().toInt() );
    this->yProfileXAxis = QVector<double>( ui->lblSizeX->text().toInt() );
    double maxValueX, maxValueY;
    int i;

    for(i = 0; i < length; i++) {
        if(i < ui->lblSizeY->text().toInt())
            this->xProfileXAxis[i] = i;
        if(i < ui->lblSizeX->text().toInt())
            this->yProfileXAxis[i] = i;

        rawValue = rawData[i].toInt();
        this->buffer[i] = static_cast<unsigned char>(rawValue);
        this->yProfile[i % ui->lblSizeX->text().toInt()] += rawValue;

        if(i > 0 && i % ui->lblSizeX->text().toInt() == 0) {
            xProfile[i / ui->lblSizeX->text().toInt() - 1] = sum;
            sum = 0;
        }
        sum += rawData[i].toInt();
    }

    QImage image(this->buffer, ui->lblSizeX->text().toInt(), ui->lblSizeY->text().toInt(), QImage::Format_Grayscale8);
    ui->lblImage->setPixmap(QPixmap::fromImage(image));

    if(!ui->cbEnableFitting->isChecked())
        return;

    maxValueX = *std::max_element(this->xProfile.constBegin(), this->xProfile.constEnd());
    maxValueY = *std::max_element(this->yProfile.constBegin(), this->yProfile.constEnd());
    std::vector<double> xProfileParameters = curve_fit(this->xProfileXAxis.toStdVector(), this->xProfile.toStdVector(), {1, (double) this->xProfile.indexOf(maxValueX), 18, 0});
    std::vector<double> yProfileParameters = curve_fit(this->yProfileXAxis.toStdVector(), this->yProfile.toStdVector(), {1, (double) this->yProfile.indexOf(maxValueY), 7, 0});

    for(i = 0; i < this->xProfile.size(); i++) {
        this->xProfileFit.push_back(gaussian(xProfileParameters[0], xProfileParameters[1], xProfileParameters[2], xProfileParameters[3], i));
    }
    for(i = 0; i < this->yProfile.size(); i++) {
        this->yProfileFit.push_back(gaussian(yProfileParameters[0], yProfileParameters[1], yProfileParameters[2], yProfileParameters[3], i));
    }

    ui->plotProfileX->graph(0)->setData(this->xProfileXAxis, this->xProfile);
    ui->plotProfileX->graph(1)->setData(this->xProfileXAxis, this->xProfileFit);
    ui->plotProfileX->rescaleAxes();
    ui->plotProfileX->replot();

    ui->plotProfileY->graph(0)->setData(this->yProfileXAxis, this->yProfile);
    ui->plotProfileY->graph(1)->setData(this->yProfileXAxis, this->yProfileFit);
    ui->plotProfileY->rescaleAxes();
    ui->plotProfileY->replot();

    this->xProfile.clear();
    this->yProfile.clear();
    this->xProfile.squeeze();
    this->yProfile.squeeze();
    this->xProfileXAxis.clear();
    this->yProfileXAxis.clear();
    this->xProfileXAxis.squeeze();
    this->yProfileXAxis.squeeze();
    delete [] this->buffer;

    double sigma_y = qSqrt(qFabs(yProfileParameters[2] / 2.0)) * 35.211 / 1E6;
    double beam_size_y = qSqrt( sigma_y * sigma_y - R_DISTANCE * R_DISTANCE * ( PH_DIVERGE * PH_DIVERGE + EL_DIVERGE * EL_DIVERGE ) );
    ui->lblVSize->setText(QString::number(beam_size_y));
    ui->lblFitA->setText(QString::number(yProfileParameters[0]));
    ui->lblFitB->setText(QString::number(yProfileParameters[1]));
    ui->lblFitC->setText(QString::number(yProfileParameters[2]));
    ui->lblFitD->setText(QString::number(yProfileParameters[3]));
}

void main_window::on_btnExpert_clicked()
{
    QProcess* p = new QProcess;
    QFile::copy(":/cam-gui.sh", "/tmp/cam-gui.sh");
    p->start(QString("bash /tmp/cam-gui.sh %1 %2").arg(this->prefix, "CAM"));
}

void main_window::closeEvent(QCloseEvent *event)
{
    ui->btnStop->click();
    QMainWindow::closeEvent(event);
}
