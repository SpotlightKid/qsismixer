#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>

void help();
void version();

//static const char * filename = "cfg.6i6";
const char* device = "hw:USB";

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    QApplication::setApplicationName("Qt Sixisix Mixer");
    QApplication::setApplicationVersion("\nversion 0.2.2\n07 Dec 2017");
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt Sixisix Mixer\nMixer GUI for controlling the Focusrite Scarlett 6i6");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
           // set alsa device handle
           {{"D","device"}, QApplication::translate("main","Use alsa device handle <device> (default hw:USB)"),
                            QApplication::translate("main","device")},
           {{"d","dump"}, QApplication::translate("main","Dump current configuration to <filename>"),
                            QApplication::translate("main","filename")},
           {{"l","load"}, QApplication::translate("main","Load configuration from <filename>"),
                            QApplication::translate("main","filename")},
                      });
    parser.process(a);
    char * tmp_device = NULL;
    if(parser.isSet("device")){
        QByteArray userDevice = parser.value("device").toLocal8Bit();

        tmp_device = new char[userDevice.size() + 1];
        strcpy(tmp_device, userDevice.data());
        device = tmp_device;
    }
    MainWindow w;
    if(parser.isSet("dump")){
        w.saveTo(parser.value("dump"));
    }
    if(parser.isSet("load")){
        w.loadFrom(parser.value("load"));
    }
    w.show();
    return a.exec();
}
