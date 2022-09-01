#include "main_window.h"
#include "ui_main_window.h"

#include <iostream>
#include <QThread>

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

    for(int i = 0; i < length; i++)
        this->buffer[i] = static_cast<unsigned char>(rawData[i].toInt());

    QImage image(this->buffer, ui->lblSizeX->text().toInt(), ui->lblSizeY->text().toInt(), QImage::Format_Grayscale8);
    ui->lblImage->setPixmap(QPixmap::fromImage(image));
    delete [] this->buffer;
}
