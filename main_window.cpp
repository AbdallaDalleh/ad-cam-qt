#include "main_window.h"
#include "ui_main_window.h"

#include <iostream>

main_window::main_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::main_window)
{
    ui->setupUi(this);
    this->prefix = "SRC06-DI-IAXD:";
    this->group = this->prefix + "CAM";

    SET_GROUP(QELabel);
    SET_GROUP(QENumericEdit);
    SET_GROUP(QEComboBox);
    SET_GROUP(QEPushButton);

    this->waveform = new QEpicsPV(this->prefix + ":image1:ArrayData");
    QObject::connect(this->waveform, SIGNAL(valueChanged(QVariant)), this, SLOT(onWaveformReceived(QVariant)));
}

main_window::~main_window()
{
    delete ui;
}

void main_window::onWaveformReceived(QVariant data)
{
    QImage image((uchar*) data.constData(), ui->lblSizeX->text().toInt(), ui->lblSizeY->text().toInt(), QImage::Format_Mono);
    ui->lblImage->setPixmap(QPixmap::fromImage(image));
    std::cout << "Image received" << std::endl;
}
