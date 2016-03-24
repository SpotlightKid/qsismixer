#include "qthread.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qmenubar.h"

const char *mixerEnumValues[] = {"Off", "PCM 1", "PCM 2", "PCM 3", "PCM 4", "PCM 5", "PCM 6", "PCM 7", "PCM 8", "PCM 9", "PCM 10", "PCM 11", "PCM 12",
                              "Analog 1", "Analog 2", "Analog 3", "Analog 4", "SPDIF 1", "SPDIF 2", "Mix A", "Mix B", "Mix C", "Mix D", "Mix E", "Mix F", "Mix G", "Mix H"};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow){
    ui->setupUi(this);
    for(int i=0; i<27; ++i){
        ui->combo_out_src_1->addItem(mixerEnumValues[i]);
        ui->combo_out_src_2->addItem(mixerEnumValues[i]);
        ui->combo_out_src_3->addItem(mixerEnumValues[i]);
        ui->combo_out_src_4->addItem(mixerEnumValues[i]);
        ui->combo_out_src_5->addItem(mixerEnumValues[i]);
        ui->combo_out_src_6->addItem(mixerEnumValues[i]);
        ui->combo_in_src_1->addItem(mixerEnumValues[i]);
        ui->combo_in_src_2->addItem(mixerEnumValues[i]);
        ui->combo_in_src_3->addItem(mixerEnumValues[i]);
        ui->combo_in_src_4->addItem(mixerEnumValues[i]);
        ui->combo_in_src_5->addItem(mixerEnumValues[i]);
        ui->combo_in_src_6->addItem(mixerEnumValues[i]);
    }
    // combo matrices only have items "Off" through "SPDIF 2"
    for(int i=0; i<19; ++i){
        ui->combo_matrix_in_1->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_2->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_3->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_4->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_5->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_6->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_7->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_8->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_9->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_10->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_11->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_12->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_13->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_14->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_15->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_16->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_17->addItem(mixerEnumValues[i]);
        ui->combo_matrix_in_18->addItem(mixerEnumValues[i]);
    }
    mixctrl.vol_master[0] = ui->slider_master_L;
    mixctrl.vol_master[1] = ui->slider_master_R;
    mixctrl.vol_master_mute = ui->check_master_mute_1;
    mixctrl.vol_master_link = ui->check_master_link_12;
    mixctrl.vol_out[0] = ui->slider_output_1;
    mixctrl.vol_out[1] = ui->slider_output_2;
    mixctrl.vol_out[2] = ui->slider_output_3;
    mixctrl.vol_out[3] = ui->slider_output_4;
    mixctrl.vol_out[4] = ui->slider_output_5;
    mixctrl.vol_out[5] = ui->slider_output_6;
    mixctrl.vol_out_mute[0] = ui->check_out_mute_1;
    mixctrl.vol_out_mute[1] = ui->check_out_mute_2;
    mixctrl.vol_out_mute[2] = ui->check_out_mute_3;
    mixctrl.vol_out_mute[3] = ui->check_out_mute_4;
    mixctrl.vol_out_mute[4] = ui->check_out_mute_5;
    mixctrl.vol_out_mute[5] = ui->check_out_mute_6;
    mixctrl.vol_out_link[0] = ui->check_out_link_12;
    mixctrl.vol_out_link[1] = ui->check_out_link_34;
    mixctrl.vol_out_link[2] = ui->check_out_link_56;
    mixctrl.out_src[0] = ui->combo_out_src_1;
    mixctrl.out_src[1] = ui->combo_out_src_2;
    mixctrl.out_src[2] = ui->combo_out_src_3;
    mixctrl.out_src[3] = ui->combo_out_src_4;
    mixctrl.out_src[4] = ui->combo_out_src_5;
    mixctrl.out_src[5] = ui->combo_out_src_6;
    mixctrl.in_src[0]  = ui->combo_in_src_1;
    mixctrl.in_src[1]  = ui->combo_in_src_2;
    mixctrl.in_src[2]  = ui->combo_in_src_3;
    mixctrl.in_src[3]  = ui->combo_in_src_4;
    mixctrl.in_src[4]  = ui->combo_in_src_5;
    mixctrl.in_src[5]  = ui->combo_in_src_6;
    mixctrl.in_imp[0] = ui->rad_imp_hiz_1;
    mixctrl.in_imp[1] = ui->rad_imp_hiz_2;
    mixctrl.in_pad[0] = ui->rad_pad_on_1;
    mixctrl.in_pad[1] = ui->rad_pad_on_2;
    mixctrl.in_pad[2] = ui->rad_pad_on_3;
    mixctrl.in_pad[3] = ui->rad_pad_on_4;

    mixctrl.mtx_src[0] = ui->combo_matrix_in_1;
    mixctrl.mtx_src[1] = ui->combo_matrix_in_2;
    mixctrl.mtx_src[2] = ui->combo_matrix_in_3;
    mixctrl.mtx_src[3] = ui->combo_matrix_in_4;
    mixctrl.mtx_src[4] = ui->combo_matrix_in_5;
    mixctrl.mtx_src[5] = ui->combo_matrix_in_6;
    mixctrl.mtx_src[6] = ui->combo_matrix_in_7;
    mixctrl.mtx_src[7] = ui->combo_matrix_in_8;
    mixctrl.mtx_src[8] = ui->combo_matrix_in_9;
    mixctrl.mtx_src[9] = ui->combo_matrix_in_10;
    mixctrl.mtx_src[10] = ui->combo_matrix_in_11;
    mixctrl.mtx_src[11] = ui->combo_matrix_in_12;
    mixctrl.mtx_src[12] = ui->combo_matrix_in_13;
    mixctrl.mtx_src[13] = ui->combo_matrix_in_14;
    mixctrl.mtx_src[14] = ui->combo_matrix_in_15;
    mixctrl.mtx_src[15] = ui->combo_matrix_in_16;
    mixctrl.mtx_src[16] = ui->combo_matrix_in_17;
    mixctrl.mtx_src[17] = ui->combo_matrix_in_18;

    mixctrl.mtx_vol[0][0] = ui->slider_mtx_a_1;
    mixctrl.mtx_vol[0][1] = ui->slider_mtx_b_1;
    mixctrl.mtx_vol[0][2] = ui->slider_mtx_c_1;
    mixctrl.mtx_vol[0][3] = ui->slider_mtx_d_1;
    mixctrl.mtx_vol[0][4] = ui->slider_mtx_e_1;
    mixctrl.mtx_vol[0][5] = ui->slider_mtx_f_1;
    mixctrl.mtx_vol[0][6] = ui->slider_mtx_g_1;
    mixctrl.mtx_vol[0][7] = ui->slider_mtx_h_1;

    mixctrl.mtx_vol[1][0] = ui->slider_mtx_a_2;
    mixctrl.mtx_vol[1][1] = ui->slider_mtx_b_2;
    mixctrl.mtx_vol[1][2] = ui->slider_mtx_c_2;
    mixctrl.mtx_vol[1][3] = ui->slider_mtx_d_2;
    mixctrl.mtx_vol[1][4] = ui->slider_mtx_e_2;
    mixctrl.mtx_vol[1][5] = ui->slider_mtx_f_2;
    mixctrl.mtx_vol[1][6] = ui->slider_mtx_g_2;
    mixctrl.mtx_vol[1][7] = ui->slider_mtx_h_2;

    mixctrl.mtx_vol[2][0] = ui->slider_mtx_a_3;
    mixctrl.mtx_vol[2][1] = ui->slider_mtx_b_3;
    mixctrl.mtx_vol[2][2] = ui->slider_mtx_c_3;
    mixctrl.mtx_vol[2][3] = ui->slider_mtx_d_3;
    mixctrl.mtx_vol[2][4] = ui->slider_mtx_e_3;
    mixctrl.mtx_vol[2][5] = ui->slider_mtx_f_3;
    mixctrl.mtx_vol[2][6] = ui->slider_mtx_g_3;
    mixctrl.mtx_vol[2][7] = ui->slider_mtx_h_3;

    mixctrl.mtx_vol[3][0] = ui->slider_mtx_a_4;
    mixctrl.mtx_vol[3][1] = ui->slider_mtx_b_4;
    mixctrl.mtx_vol[3][2] = ui->slider_mtx_c_4;
    mixctrl.mtx_vol[3][3] = ui->slider_mtx_d_4;
    mixctrl.mtx_vol[3][4] = ui->slider_mtx_e_4;
    mixctrl.mtx_vol[3][5] = ui->slider_mtx_f_4;
    mixctrl.mtx_vol[3][6] = ui->slider_mtx_g_4;
    mixctrl.mtx_vol[3][7] = ui->slider_mtx_h_4;

    mixctrl.mtx_vol[4][0] = ui->slider_mtx_a_5;
    mixctrl.mtx_vol[4][1] = ui->slider_mtx_b_5;
    mixctrl.mtx_vol[4][2] = ui->slider_mtx_c_5;
    mixctrl.mtx_vol[4][3] = ui->slider_mtx_d_5;
    mixctrl.mtx_vol[4][4] = ui->slider_mtx_e_5;
    mixctrl.mtx_vol[4][5] = ui->slider_mtx_f_5;
    mixctrl.mtx_vol[4][6] = ui->slider_mtx_g_5;
    mixctrl.mtx_vol[4][7] = ui->slider_mtx_h_5;

    mixctrl.mtx_vol[5][0] = ui->slider_mtx_a_6;
    mixctrl.mtx_vol[5][1] = ui->slider_mtx_b_6;
    mixctrl.mtx_vol[5][2] = ui->slider_mtx_c_6;
    mixctrl.mtx_vol[5][3] = ui->slider_mtx_d_6;
    mixctrl.mtx_vol[5][4] = ui->slider_mtx_e_6;
    mixctrl.mtx_vol[5][5] = ui->slider_mtx_f_6;
    mixctrl.mtx_vol[5][6] = ui->slider_mtx_g_6;
    mixctrl.mtx_vol[5][7] = ui->slider_mtx_h_6;


    mixctrl.mtx_vol[6][0] = ui->slider_mtx_a_7;
    mixctrl.mtx_vol[6][1] = ui->slider_mtx_b_7;
    mixctrl.mtx_vol[6][2] = ui->slider_mtx_c_7;
    mixctrl.mtx_vol[6][3] = ui->slider_mtx_d_7;
    mixctrl.mtx_vol[6][4] = ui->slider_mtx_e_7;
    mixctrl.mtx_vol[6][5] = ui->slider_mtx_f_7;
    mixctrl.mtx_vol[6][6] = ui->slider_mtx_g_7;
    mixctrl.mtx_vol[6][7] = ui->slider_mtx_h_7;

    mixctrl.mtx_vol[7][0] = ui->slider_mtx_a_8;
    mixctrl.mtx_vol[7][1] = ui->slider_mtx_b_8;
    mixctrl.mtx_vol[7][2] = ui->slider_mtx_c_8;
    mixctrl.mtx_vol[7][3] = ui->slider_mtx_d_8;
    mixctrl.mtx_vol[7][4] = ui->slider_mtx_e_8;
    mixctrl.mtx_vol[7][5] = ui->slider_mtx_f_8;
    mixctrl.mtx_vol[7][6] = ui->slider_mtx_g_8;
    mixctrl.mtx_vol[7][7] = ui->slider_mtx_h_8;

    mixctrl.mtx_vol[8][0] = ui->slider_mtx_a_9;
    mixctrl.mtx_vol[8][1] = ui->slider_mtx_b_9;
    mixctrl.mtx_vol[8][2] = ui->slider_mtx_c_9;
    mixctrl.mtx_vol[8][3] = ui->slider_mtx_d_9;
    mixctrl.mtx_vol[8][4] = ui->slider_mtx_e_9;
    mixctrl.mtx_vol[8][5] = ui->slider_mtx_f_9;
    mixctrl.mtx_vol[8][6] = ui->slider_mtx_g_9;
    mixctrl.mtx_vol[8][7] = ui->slider_mtx_h_9;

    mixctrl.mtx_vol[9][0] = ui->slider_mtx_a_10;
    mixctrl.mtx_vol[9][1] = ui->slider_mtx_b_10;
    mixctrl.mtx_vol[9][2] = ui->slider_mtx_c_10;
    mixctrl.mtx_vol[9][3] = ui->slider_mtx_d_10;
    mixctrl.mtx_vol[9][4] = ui->slider_mtx_e_10;
    mixctrl.mtx_vol[9][5] = ui->slider_mtx_f_10;
    mixctrl.mtx_vol[9][6] = ui->slider_mtx_g_10;
    mixctrl.mtx_vol[9][7] = ui->slider_mtx_h_10;

    mixctrl.mtx_vol[10][0] = ui->slider_mtx_a_11;
    mixctrl.mtx_vol[10][1] = ui->slider_mtx_b_11;
    mixctrl.mtx_vol[10][2] = ui->slider_mtx_c_11;
    mixctrl.mtx_vol[10][3] = ui->slider_mtx_d_11;
    mixctrl.mtx_vol[10][4] = ui->slider_mtx_e_11;
    mixctrl.mtx_vol[10][5] = ui->slider_mtx_f_11;
    mixctrl.mtx_vol[10][6] = ui->slider_mtx_g_11;
    mixctrl.mtx_vol[10][7] = ui->slider_mtx_h_11;

    mixctrl.mtx_vol[11][0] = ui->slider_mtx_a_12;
    mixctrl.mtx_vol[11][1] = ui->slider_mtx_b_12;
    mixctrl.mtx_vol[11][2] = ui->slider_mtx_c_12;
    mixctrl.mtx_vol[11][3] = ui->slider_mtx_d_12;
    mixctrl.mtx_vol[11][4] = ui->slider_mtx_e_12;
    mixctrl.mtx_vol[11][5] = ui->slider_mtx_f_12;
    mixctrl.mtx_vol[11][6] = ui->slider_mtx_g_12;
    mixctrl.mtx_vol[11][7] = ui->slider_mtx_h_12;


    mixctrl.mtx_vol[12][0] = ui->slider_mtx_a_13;
    mixctrl.mtx_vol[12][1] = ui->slider_mtx_b_13;
    mixctrl.mtx_vol[12][2] = ui->slider_mtx_c_13;
    mixctrl.mtx_vol[12][3] = ui->slider_mtx_d_13;
    mixctrl.mtx_vol[12][4] = ui->slider_mtx_e_13;
    mixctrl.mtx_vol[12][5] = ui->slider_mtx_f_13;
    mixctrl.mtx_vol[12][6] = ui->slider_mtx_g_13;
    mixctrl.mtx_vol[12][7] = ui->slider_mtx_h_13;

    mixctrl.mtx_vol[13][0] = ui->slider_mtx_a_14;
    mixctrl.mtx_vol[13][1] = ui->slider_mtx_b_14;
    mixctrl.mtx_vol[13][2] = ui->slider_mtx_c_14;
    mixctrl.mtx_vol[13][3] = ui->slider_mtx_d_14;
    mixctrl.mtx_vol[13][4] = ui->slider_mtx_e_14;
    mixctrl.mtx_vol[13][5] = ui->slider_mtx_f_14;
    mixctrl.mtx_vol[13][6] = ui->slider_mtx_g_14;
    mixctrl.mtx_vol[13][7] = ui->slider_mtx_h_14;

    mixctrl.mtx_vol[14][0] = ui->slider_mtx_a_15;
    mixctrl.mtx_vol[14][1] = ui->slider_mtx_b_15;
    mixctrl.mtx_vol[14][2] = ui->slider_mtx_c_15;
    mixctrl.mtx_vol[14][3] = ui->slider_mtx_d_15;
    mixctrl.mtx_vol[14][4] = ui->slider_mtx_e_15;
    mixctrl.mtx_vol[14][5] = ui->slider_mtx_f_15;
    mixctrl.mtx_vol[14][6] = ui->slider_mtx_g_15;
    mixctrl.mtx_vol[14][7] = ui->slider_mtx_h_15;

    mixctrl.mtx_vol[15][0] = ui->slider_mtx_a_16;
    mixctrl.mtx_vol[15][1] = ui->slider_mtx_b_16;
    mixctrl.mtx_vol[15][2] = ui->slider_mtx_c_16;
    mixctrl.mtx_vol[15][3] = ui->slider_mtx_d_16;
    mixctrl.mtx_vol[15][4] = ui->slider_mtx_e_16;
    mixctrl.mtx_vol[15][5] = ui->slider_mtx_f_16;
    mixctrl.mtx_vol[15][6] = ui->slider_mtx_g_16;
    mixctrl.mtx_vol[15][7] = ui->slider_mtx_h_16;

    mixctrl.mtx_vol[16][0] = ui->slider_mtx_a_17;
    mixctrl.mtx_vol[16][1] = ui->slider_mtx_b_17;
    mixctrl.mtx_vol[16][2] = ui->slider_mtx_c_17;
    mixctrl.mtx_vol[16][3] = ui->slider_mtx_d_17;
    mixctrl.mtx_vol[16][4] = ui->slider_mtx_e_17;
    mixctrl.mtx_vol[16][5] = ui->slider_mtx_f_17;
    mixctrl.mtx_vol[16][6] = ui->slider_mtx_g_17;
    mixctrl.mtx_vol[16][7] = ui->slider_mtx_h_17;

    mixctrl.mtx_vol[17][0] = ui->slider_mtx_a_18;
    mixctrl.mtx_vol[17][1] = ui->slider_mtx_b_18;
    mixctrl.mtx_vol[17][2] = ui->slider_mtx_c_18;
    mixctrl.mtx_vol[17][3] = ui->slider_mtx_d_18;
    mixctrl.mtx_vol[17][4] = ui->slider_mtx_e_18;
    mixctrl.mtx_vol[17][5] = ui->slider_mtx_f_18;
    mixctrl.mtx_vol[17][6] = ui->slider_mtx_g_18;
    mixctrl.mtx_vol[17][7] = ui->slider_mtx_h_18;

    for(int i=0; i<18; ++i){
        for(int j=0; j<8; ++j){
            mixctrl.mtx_vol[i][j]->setTracking(1);
            mixctrl.mtx_vol[i][j]->setMaximum(128);
        }
        if(i>=6) break;
        mixctrl.vol_out[i]->setTracking(1);
        if(i>=2) break;
        mixctrl.vol_master[i]->setTracking(1);
    }
    mixer = new MixSis(&mixctrl, device, this);

    createMenu();

}

MainWindow::~MainWindow()
{
    delete mixer;
    delete ui;
}

bool MainWindow::event(QEvent *ev){
    if(ev->type() == QEvent::Quit){
        exit(-1);
        return false;
    }
    return QWidget::event(ev);
}

int MainWindow::loadFrom(const char* filename){
    QString filen(filename);
    return mixctrl.load_from(filen);
}

int MainWindow::saveTo(const char* filename){
    QString filen(filename);
    return mixctrl.save_to(filen);
}


void MainWindow::createMenu(){
    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, this, [=](){
       mixctrl.save_to_dialog(this);
    });

    loadAct = new QAction(tr("&Load"), this);
    loadAct->setShortcuts(QKeySequence::Open);
    connect(loadAct, &QAction::triggered, this, [=](){
        mixctrl.load_from_dialog(this);
    });
    exitAct = new QAction(tr("&Exit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, [=](){
        exit(0);
    });
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(saveAct);
    fileMenu->addAction(loadAct);
    fileMenu->addAction(exitAct);
}
