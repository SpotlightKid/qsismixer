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
    QApplication::setApplicationVersion("\nversion 0.2\n17 Dec 2016");
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
           {"no-reset", QApplication::translate("main","Don't reset mixer controls to the values reported by the alsa backend")},
                      });
    MainWindow w;
    parser.process(a);
    if(parser.isSet("device")){
        device = parser.value("device").toLatin1().data();
    }
    if(parser.isSet("dump")){
        w.saveTo(parser.value("dump"));
    }
    if(parser.isSet("load")){
        w.loadFrom(parser.value("load"));
    }
    if(!parser.isSet("no-reset")){
        w.doReset();
    }
    w.show();
    return a.exec();
}
