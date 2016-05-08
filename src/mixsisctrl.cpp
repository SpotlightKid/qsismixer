#include "qdatastream.h"
#include "qfile.h"
#include "qtextstream.h"
#include <QStandardPaths>
#include <QDir>
#include "qfiledialog.h"
#include "qobject.h"
#include "mixsisctrl.h"

const char *magicn = "QSIS";

bool MixSisCtrl::numidIsVolume(alsa_numid test){
    if(test == MSTR_VOL) return 1;
    if(test == OUT_VOL_12 ) return 1;
    if(test == OUT_VOL_34) return 1;
    if(test == OUT_VOL_56) return 1;
    if(test >= MATRIX_A_1 && test <= MATRIX_H_1) return 1;
    if(test >= MATRIX_A_2 && test <= MATRIX_H_2) return 1;
    if(test >= MATRIX_A_3 && test <= MATRIX_H_3) return 1;
    if(test >= MATRIX_A_4 && test <= MATRIX_H_4) return 1;
    if(test >= MATRIX_A_5 && test <= MATRIX_H_5) return 1;
    if(test >= MATRIX_A_6 && test <= MATRIX_H_6) return 1;
    if(test >= MATRIX_A_7 && test <= MATRIX_H_7) return 1;
    if(test >= MATRIX_A_8 && test <= MATRIX_H_8) return 1;
    if(test >= MATRIX_A_9 && test <= MATRIX_H_9) return 1;
    if(test >= MATRIX_A_10 && test <= MATRIX_H_10) return 1;
    if(test >= MATRIX_A_11 && test <= MATRIX_H_11) return 1;
    if(test >= MATRIX_A_12 && test <= MATRIX_H_12) return 1;
    if(test >= MATRIX_A_13 && test <= MATRIX_H_13) return 1;
    if(test >= MATRIX_A_14 && test <= MATRIX_H_14) return 1;
    if(test >= MATRIX_A_15 && test <= MATRIX_H_15) return 1;
    if(test >= MATRIX_A_16 && test <= MATRIX_H_16) return 1;
    if(test >= MATRIX_A_17 && test <= MATRIX_H_17) return 1;
    if(test >= MATRIX_A_18 && test <= MATRIX_H_18) return 1;
    return 0;
}

int MixSisCtrl::save_to_dialog(QWidget *context){
    QString directory = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QString filen = QFileDialog::getSaveFileName(context, context->tr("Save Configuration As..."), directory.append(context->tr("/qsismix.6i6")), context->tr("Sixisix Configs (*.6i6)"));
    //QString filen = QString("qsismix.cfg");
    return save_to(filen);
}

int MixSisCtrl::save_to(QString &filename){
    QFile file(filename);
    if(!file.open( QIODevice::WriteOnly )){
        fprintf(stderr, "%s\n", file.errorString().toLocal8Bit().constData());
        fprintf(stderr,"unable to write config to file \"%s\"\n", filename.toLocal8Bit().constData());
        return -1;
    }
    else{
        fprintf(stderr, "successfully opened file \"%s\"\n", filename.toLocal8Bit().constData());
    }
    QDataStream cfg(&file);
    cfg.writeRawData(magicn, 4);
    for(int i=0; i<2; ++i){
        cfg << (qint32)vol_master[i]->value();
        cfg << (qint32)in_imp[i*2+1]->isChecked();
    }
    cfg << (qint32) vol_master_mute->isChecked();
    cfg << (qint32) vol_master_link->isChecked();
    for(int i=0; i<6; ++i){
        cfg << (qint32) vol_out[i]->value();
        cfg << (qint32) vol_out_mute[i]->isChecked();
        cfg << (qint32) out_src[i]->currentIndex();
        cfg << (qint32) in_src[i]->currentIndex();
    }
    for(int i=0; i<3; ++i){
        cfg << (qint32) vol_out_link[i]->isChecked();
    }
    for(int i=0; i<4; ++i){
        cfg << (qint32) in_pad[i*2+1]->isChecked();
    }
    for(int i=0; i<18; ++i){
        cfg << (qint32) mtx_src[i]->currentIndex();
        for(int j=0; j<8; ++j){
            cfg << (qint32) mtx_vol[i][j]->value();
        }
    }
    file.close();
    return 0;
}

int MixSisCtrl::load_from_dialog(QWidget *context){
    QString directory = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QString filen = QFileDialog::getOpenFileName(context, context->tr("Load Configuration From..."), directory.append("/qsismix.6i6"), context->tr("Sixisix Configs (*.6i6)"));
    return load_from(filen);
}

int MixSisCtrl::load_from(QString &filename){
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    if(!file.isOpen()){
        fprintf(stderr, "unable to read from config file \"%s\"\n",filename.toLocal8Bit().constData());
        return -1;
    }
    else{
        fprintf(stderr, "successfully opened file \"%s\"\n", filename.toLocal8Bit().constData());
    }
    QDataStream cfg(&file);
    for(int i=0;i<4;++i){
        qint8 n;
        cfg >> n;
        if(n != magicn[i]){
            fprintf(stderr, "\"%s\" is not a QSIS config file, aborting...\n", filename.toLocal8Bit().constData());
            return -1;
        }
    }
    qint32 tmp;
    for(int i=0; i<2; ++i){
        cfg >> tmp;
        vol_master[i]->setValue(tmp);
        cfg >> tmp;
        in_imp[i*2+1]->setChecked(tmp);
    }
    cfg >> tmp;
    vol_master_mute->setChecked(tmp);
    cfg >> tmp;
    vol_master_link->setChecked(tmp);
    for(int i=0; i<6; ++i){
        cfg >> tmp;
        vol_out[i]->setValue(tmp);
        cfg >> tmp;
        vol_out_mute[i]->setChecked(tmp);
        cfg >> tmp;
        out_src[i]->setCurrentIndex(tmp);
        cfg >> tmp;
        in_src[i]->setCurrentIndex(tmp);
    }
    for(int i=0; i<3; ++i){
        cfg >> tmp;
        vol_out_link[i]->setChecked(tmp);
    }
    for(int i=0; i<4; ++i){
        cfg >> tmp;
        in_pad[i*2+1]->setChecked(tmp);
    }
    for(int i=0; i<18; ++i){
        cfg >> tmp;
        mtx_src[i]->setCurrentIndex(tmp);
        for(int j=0; j<8; ++j){
            cfg >> tmp;
            mtx_vol[i][j]->setValue(tmp);
        }
    }
    file.close();
    return 0;
}

void MixSisCtrl::set(int alsa_id, int value, int idx){
    long db;
    int j = 0;
    switch((alsa_numid) alsa_id){
        case MSTR_SWITCH:
            vol_master_mute->setChecked(!value);
            break;
    case MSTR_VOL:
            db = value;
            vol_master[0]->setValue(db);
            vol_master[1]->setValue(db);
        break;
    case OUT_SWITCH_56:
        j+=2;
    case OUT_SWITCH_34:
        j+=2;
    case OUT_SWITCH_12:
        vol_out_mute[j+idx]->setChecked(!value);
        break;
    case OUT_VOL_56:
        j+=2;
    case OUT_VOL_34:
        j+=2;
    case OUT_VOL_12:
        vol_out[j+idx]->setValue(value);
        break;
    case OUT_6_SRC:
        ++j;
    case OUT_5_SRC:
        ++j;
    case OUT_4_SRC:
        ++j;
    case OUT_3_SRC:
        ++j;
    case OUT_2_SRC:
        ++j;
    case OUT_1_SRC:
        out_src[j]->setCurrentIndex(value);
        break;
    case IN_IMP_2:
        ++j;
    case IN_IMP_1:
        in_imp[2*j + value]->setChecked(1);
        break;
    case IN_PAD_4:
        ++j;
    case IN_PAD_3:
        ++j;
    case IN_PAD_2:
        ++j;
    case IN_PAD_1:
        in_pad[2*j + value]->setChecked(1);
        break;
    case INPUT_ROUTE_6:
        ++j;
    case INPUT_ROUTE_5:
        ++j;
    case INPUT_ROUTE_4:
        ++j;
    case INPUT_ROUTE_3:
        ++j;
    case INPUT_ROUTE_2:
        ++j;
    case INPUT_ROUTE_1:
        in_src[j]->setCurrentIndex(value);
        break;
    case MATRIX_ROUTE_18:
        ++j;
    case MATRIX_ROUTE_17:
        ++j;
    case MATRIX_ROUTE_16:
        ++j;
    case MATRIX_ROUTE_15:
        ++j;
    case MATRIX_ROUTE_14:
        ++j;
    case MATRIX_ROUTE_13:
        ++j;
    case MATRIX_ROUTE_12:
        ++j;
    case MATRIX_ROUTE_11:
        ++j;
    case MATRIX_ROUTE_10:
        ++j;
    case MATRIX_ROUTE_9:
        ++j;
    case MATRIX_ROUTE_8:
        ++j;
    case MATRIX_ROUTE_7:
        ++j;
    case MATRIX_ROUTE_6:
        ++j;
    case MATRIX_ROUTE_5:
        ++j;
    case MATRIX_ROUTE_4:
        ++j;
    case MATRIX_ROUTE_3:
        ++j;
    case MATRIX_ROUTE_2:
        ++j;
    case MATRIX_ROUTE_1:
        mtx_src[j]->setCurrentIndex(value);
        break;
    case MATRIX_A_18:
        ++j;
    case MATRIX_A_17:
        ++j;
    case MATRIX_A_16:
        ++j;
    case MATRIX_A_15:
        ++j;
    case MATRIX_A_14:
        ++j;
    case MATRIX_A_13:
        ++j;
    case MATRIX_A_12:
        ++j;
    case MATRIX_A_11:
        ++j;
    case MATRIX_A_10:
        ++j;
    case MATRIX_A_9:
        ++j;
    case MATRIX_A_8:
        ++j;
    case MATRIX_A_7:
        ++j;
    case MATRIX_A_6:
        ++j;
    case MATRIX_A_5:
        ++j;
    case MATRIX_A_4:
        ++j;
    case MATRIX_A_3:
        ++j;
    case MATRIX_A_2:
        ++j;
    case MATRIX_A_1:
        mtx_vol[j][0]->setValue(value);
        break;
    case MATRIX_B_18:
        ++j;
    case MATRIX_B_17:
        ++j;
    case MATRIX_B_16:
        ++j;
    case MATRIX_B_15:
        ++j;
    case MATRIX_B_14:
        ++j;
    case MATRIX_B_13:
        ++j;
    case MATRIX_B_12:
        ++j;
    case MATRIX_B_11:
        ++j;
    case MATRIX_B_10:
        ++j;
    case MATRIX_B_9:
        ++j;
    case MATRIX_B_8:
        ++j;
    case MATRIX_B_7:
        ++j;
    case MATRIX_B_6:
        ++j;
    case MATRIX_B_5:
        ++j;
    case MATRIX_B_4:
        ++j;
    case MATRIX_B_3:
        ++j;
    case MATRIX_B_2:
        ++j;
    case MATRIX_B_1:
        mtx_vol[j][1]->setValue(value);
        break;
    case MATRIX_C_18:
        ++j;
    case MATRIX_C_17:
        ++j;
    case MATRIX_C_16:
        ++j;
    case MATRIX_C_15:
        ++j;
    case MATRIX_C_14:
        ++j;
    case MATRIX_C_13:
        ++j;
    case MATRIX_C_12:
        ++j;
    case MATRIX_C_11:
        ++j;
    case MATRIX_C_10:
        ++j;
    case MATRIX_C_9:
        ++j;
    case MATRIX_C_8:
        ++j;
    case MATRIX_C_7:
        ++j;
    case MATRIX_C_6:
        ++j;
    case MATRIX_C_5:
        ++j;
    case MATRIX_C_4:
        ++j;
    case MATRIX_C_3:
        ++j;
    case MATRIX_C_2:
        ++j;
    case MATRIX_C_1:
        mtx_vol[j][2]->setValue(value);
        break;
    case MATRIX_D_18:
        ++j;
    case MATRIX_D_17:
        ++j;
    case MATRIX_D_16:
        ++j;
    case MATRIX_D_15:
        ++j;
    case MATRIX_D_14:
        ++j;
    case MATRIX_D_13:
        ++j;
    case MATRIX_D_12:
        ++j;
    case MATRIX_D_11:
        ++j;
    case MATRIX_D_10:
        ++j;
    case MATRIX_D_9:
        ++j;
    case MATRIX_D_8:
        ++j;
    case MATRIX_D_7:
        ++j;
    case MATRIX_D_6:
        ++j;
    case MATRIX_D_5:
        ++j;
    case MATRIX_D_4:
        ++j;
    case MATRIX_D_3:
        ++j;
    case MATRIX_D_2:
        ++j;
    case MATRIX_D_1:
        mtx_vol[j][3]->setValue(value);
        break;
    case MATRIX_E_18:
        ++j;
    case MATRIX_E_17:
        ++j;
    case MATRIX_E_16:
        ++j;
    case MATRIX_E_15:
        ++j;
    case MATRIX_E_14:
        ++j;
    case MATRIX_E_13:
        ++j;
    case MATRIX_E_12:
        ++j;
    case MATRIX_E_11:
        ++j;
    case MATRIX_E_10:
        ++j;
    case MATRIX_E_9:
        ++j;
    case MATRIX_E_8:
        ++j;
    case MATRIX_E_7:
        ++j;
    case MATRIX_E_6:
        ++j;
    case MATRIX_E_5:
        ++j;
    case MATRIX_E_4:
        ++j;
    case MATRIX_E_3:
        ++j;
    case MATRIX_E_2:
        ++j;
    case MATRIX_E_1:
        mtx_vol[j][4]->setValue(value);
        break;
    case MATRIX_F_18:
        ++j;
    case MATRIX_F_17:
        ++j;
    case MATRIX_F_16:
        ++j;
    case MATRIX_F_15:
        ++j;
    case MATRIX_F_14:
        ++j;
    case MATRIX_F_13:
        ++j;
    case MATRIX_F_12:
        ++j;
    case MATRIX_F_11:
        ++j;
    case MATRIX_F_10:
        ++j;
    case MATRIX_F_9:
        ++j;
    case MATRIX_F_8:
        ++j;
    case MATRIX_F_7:
        ++j;
    case MATRIX_F_6:
        ++j;
    case MATRIX_F_5:
        ++j;
    case MATRIX_F_4:
        ++j;
    case MATRIX_F_3:
        ++j;
    case MATRIX_F_2:
        ++j;
    case MATRIX_F_1:
        mtx_vol[j][5]->setValue(value);
        break;
    case MATRIX_G_18:
        ++j;
    case MATRIX_G_17:
        ++j;
    case MATRIX_G_16:
        ++j;
    case MATRIX_G_15:
        ++j;
    case MATRIX_G_14:
        ++j;
    case MATRIX_G_13:
        ++j;
    case MATRIX_G_12:
        ++j;
    case MATRIX_G_11:
        ++j;
    case MATRIX_G_10:
        ++j;
    case MATRIX_G_9:
        ++j;
    case MATRIX_G_8:
        ++j;
    case MATRIX_G_7:
        ++j;
    case MATRIX_G_6:
        ++j;
    case MATRIX_G_5:
        ++j;
    case MATRIX_G_4:
        ++j;
    case MATRIX_G_3:
        ++j;
    case MATRIX_G_2:
        ++j;
    case MATRIX_G_1:
        mtx_vol[j][6]->setValue(value);
        break;
    case MATRIX_H_18:
        ++j;
    case MATRIX_H_17:
        ++j;
    case MATRIX_H_16:
        ++j;
    case MATRIX_H_15:
        ++j;
    case MATRIX_H_14:
        ++j;
    case MATRIX_H_13:
        ++j;
    case MATRIX_H_12:
        ++j;
    case MATRIX_H_11:
        ++j;
    case MATRIX_H_10:
        ++j;
    case MATRIX_H_9:
        ++j;
    case MATRIX_H_8:
        ++j;
    case MATRIX_H_7:
        ++j;
    case MATRIX_H_6:
        ++j;
    case MATRIX_H_5:
        ++j;
    case MATRIX_H_4:
        ++j;
    case MATRIX_H_3:
        ++j;
    case MATRIX_H_2:
        ++j;
    case MATRIX_H_1:
        mtx_vol[j][7]->setValue(value);
        break;
    default:
        fprintf(stderr, "invalid alsa ID: %d\n", alsa_id);
    }
}
