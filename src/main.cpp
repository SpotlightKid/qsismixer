#include "mainwindow.h"
#include <QApplication>

void help();
void version();

static const char * filename = "cfg.6i6";
const char* device = "hw:USB";

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    int toSave = 0;
    int toLoad = 0;
    for(int i=1; i<argc; ++i){
        switch(argv[i][0]){
        case '-':
            if(argv[i][1] == 'v'){
                version();
                return 0;
            }
            else if(argv[i][1] == 'd' || argv[i][1] == 'l'){
                if( i+1 < argc ){
                    filename = argv[i+1];
                }
                if(argv[i++][1] == 'd'){
                    toSave = 1;
                    break;
                }
                else{
                    toLoad = 1;
                    break;
                }
            }
            else if(argv[i][1] == 'h'){
                help();
                return 0;
            }
            else if(argv[i][1] == 'D'){
                if(++i < argc){
                    device = argv[i];
                }
                continue;
            }
        default:
            help();
            return -1;
        }
    }
    MainWindow w;
    if(toSave){
        return w.saveTo(filename);
    }
    if(toLoad){
        return w.loadFrom(filename);
    }
    w.show();
    return a.exec();
}

void help(){
    fprintf(stdout,"%s\n%s\n\n\t%-20s%s\n\t%-20s%s\n\t%-20s%s\n\t%-20s%s\n\t%-20s%s\n",
            "Qt Sixisix Mixer",
            "Usage: qsismix [ options ]",
            "-D (device)","selects alsa device handle (default hw:USB)",
            "-d (filename)","saves (dumps) the current configuration to file given (default cfg.6i6) and then exits",
            "-l (filename)","loads a configuration from the file given (default cfg.6i6) and then exits",
            "-v","prints version information",
            "-h","prints this help file");
}

void version(){
    fprintf(stdout, "Qt Sixisix Mixer\n"
            "version 0.17\n"
            "May 08, 2016\n");
}
