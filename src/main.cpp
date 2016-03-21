#include "mainwindow.h"
#include <QApplication>

void help();
void version();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    for(int i=1; i<argc; ++i){
        switch(argv[i][0]){
        case '-':
            if(argv[i][1] == 'v'){
                version();
                return 0;
            }
            else if(argv[i][1] == 'l' || argv[i][1] == 'd'){
                if(!( i+1 < argc )){
                    help();
                    return -1;
                }
                if(argv[i++][1] == 'l'){
                    return w.loadFrom(argv[i]);
                }
                else{
                    return w.saveTo(argv[i]);
                }
            }
            else if(argv[i][1] == 'h'){
                help();
                return 0;
            }
        default:
            help();
            return -1;
        }
    }
    w.show();
    return a.exec();
}

void help(){
    fprintf(stdout, "Qt Sixisix Mixer\n"
            "Usage: qsismix [ options ]\n"
            "\n"
            "\t-l loads a configuration from the file given and then exits\n"
            "\t-d saves (dumps) the current configuration to file given and then exits\n"
            "\t-v prints version information\n"
            "\t-h prints this help file\n");
}

void version(){
    fprintf(stdout, "Qt Sixisix Mixer\n"
            "version 0.14\n"
            "March 21, 2016\n");
}
