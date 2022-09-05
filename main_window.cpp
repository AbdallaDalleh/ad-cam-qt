#include "main_window.h"
#include "ui_main_window.h"

#include <iostream>
#include <algorithm>

using namespace std;

main_window::main_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::main_window)
{
    ui->setupUi(this);
    this->prefix = "SRC06-DI-IAXD";
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
    ui->plotProfileX->xAxis->setLabel("Samples");
    ui->plotProfileX->yAxis->setLabel("Profile");
    ui->plotProfileY->addGraph();
    ui->plotProfileY->xAxis->setLabel("Samples");
    ui->plotProfileY->yAxis->setLabel("Profile");
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

    for(int i = 0; i < length; i++) {
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

    ui->plotProfileX->graph(0)->setData(this->xProfileXAxis, this->xProfile);
    ui->plotProfileY->graph(0)->setData(this->yProfileXAxis, this->yProfile);
    ui->plotProfileX->rescaleAxes();
    ui->plotProfileY->rescaleAxes();
    ui->plotProfileX->replot();
    ui->plotProfileY->replot();

    maxValueX = *std::max_element(this->xProfile.constBegin(), this->xProfile.constEnd());
    maxValueY = *std::max_element(this->yProfile.constBegin(), this->yProfile.constEnd());
    std::vector<double> xProfileParameters = curve_fit(this->xProfileXAxis.toStdVector(), this->xProfile.toStdVector(), {1, (double) this->xProfile.indexOf(maxValueX), 18, 0});
    std::vector<double> yProfileParameters = curve_fit(this->yProfileXAxis.toStdVector(), this->yProfile.toStdVector(), {1, (double) this->yProfile.indexOf(maxValueY), 7, 0});

    this->xProfile.clear();
    this->yProfile.clear();
    this->xProfile.squeeze();
    this->yProfile.squeeze();
    this->xProfileXAxis.clear();
    this->yProfileXAxis.clear();
    this->xProfileXAxis.squeeze();
    this->yProfileXAxis.squeeze();
    delete [] this->buffer;
}

void main_window::on_btnExpert_clicked()
{
    QProcess* p = new QProcess;
    QFile::copy(":/cam-gui.sh", "/tmp/cam-gui.sh");
    p->start(QString("bash /tmp/cam-gui.sh %1 %2").arg(this->prefix, "CAM"));
}

void main_window::performCurveFitting()
{

}
