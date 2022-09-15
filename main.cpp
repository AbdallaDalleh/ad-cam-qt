#include "main_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString prefix;
    if(argv[1])
        prefix = QString(argv[1]);
    else
        prefix = "SRC06-DI-IAXD";
    main_window w(prefix);
    w.show();
    return a.exec();
}
