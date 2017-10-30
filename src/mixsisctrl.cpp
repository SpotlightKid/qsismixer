#include "qfile.h"
#include "qtextstream.h"
#include <QDir>
#include "qobject.h"
#include "qtemporaryfile.h"
#include "mixsisctrl.h"

const char *magicn = "QSIS";

bool MixSisCtrl::numidIsVolume(alsa_numid test){
    if(test == MSTR_VOL) return true;
    if(test == OUT_VOL_12 ) return true;
    if(test == OUT_VOL_34) return true;
    if(test == OUT_VOL_56) return true;
    if(test >= MATRIX_1_A && test <= MATRIX_1_H) return true;
    if(test >= MATRIX_2_A && test <= MATRIX_2_H) return true;
    if(test >= MATRIX_3_A && test <= MATRIX_3_H) return true;
    if(test >= MATRIX_4_A && test <= MATRIX_4_H) return true;
    if(test >= MATRIX_5_A && test <= MATRIX_5_H) return true;
    if(test >= MATRIX_6_A && test <= MATRIX_6_H) return true;
    if(test >= MATRIX_7_A && test <= MATRIX_7_H) return true;
    if(test >= MATRIX_8_A && test <= MATRIX_8_H) return true;
    if(test >= MATRIX_9_A && test <= MATRIX_9_H) return true;
    if(test >= MATRIX_10_A && test <= MATRIX_10_H) return true;
    if(test >= MATRIX_11_A && test <= MATRIX_11_H) return true;
    if(test >= MATRIX_12_A && test <= MATRIX_12_H) return true;
    if(test >= MATRIX_13_A && test <= MATRIX_13_H) return true;
    if(test >= MATRIX_14_A && test <= MATRIX_14_H) return true;
    if(test >= MATRIX_15_A && test <= MATRIX_15_H) return true;
    if(test >= MATRIX_16_A && test <= MATRIX_16_H) return true;
    if(test >= MATRIX_17_A && test <= MATRIX_17_H) return true;
    if(test >= MATRIX_18_A && test <= MATRIX_18_H) return true;
    return false;
}

int MixSisCtrl::save_to(const QString &filename){
    QFile file(filename);
    if(!file.open( QIODevice::WriteOnly )){
        fprintf(stderr, "%s\n", file.errorString().toLocal8Bit().constData());
        fprintf(stderr,"unable to write config to file \"%s\"\n", filename.toLocal8Bit().constData());
        return -1;
    }
    else{
        fprintf(stderr, "successfully opened file \"%s\" for write\n", filename.toLocal8Bit().constData());
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
    /// best case scenario would be to instead define a QExceptionDataStream : QDataStream that throws exceptions like a modern thing
    /// or just use std::fstream and flag it to throw exceptions instead
    if(cfg.status() != QDataStream::Ok){
        fprintf(stderr, "File output error! Probably corrupted data at %s.\n", filename.toLocal8Bit().constData());
        file.close();
        return -1;
    }
    file.close();
    return 0;
}

int MixSisCtrl::load_from(const QString &filename){
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    if(!file.isOpen()){
        fprintf(stderr, "unable to read from config file \"%s\"\n",filename.toLocal8Bit().constData());
        return -1;
    }
    else{
        fprintf(stderr, "successfully opened file \"%s\" for read\n", filename.toLocal8Bit().constData());
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
    // here simply setting the controls will work, since they are already connected to alsa in MixSis
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
    // unfortunately, QT *won't* throw an exception on trying to read past eof (as far as I can tell)
    if(cfg.status() != QDataStream::Ok){
        fprintf(stderr, "File input error! Probably corrupted data at %s.\n"
                "It is also very likely that qsis settings are corrupted.\n", filename.toLocal8Bit().constData());
        file.close();
        return -1;
    }
    file.close();
    return 0;
}

/// sets the qt control corresponding to alsa_id,idx to a certain value
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
    case MATRIX_18_A:
        ++j;
    case MATRIX_17_A:
        ++j;
    case MATRIX_16_A:
        ++j;
    case MATRIX_15_A:
        ++j;
    case MATRIX_14_A:
        ++j;
    case MATRIX_13_A:
        ++j;
    case MATRIX_12_A:
        ++j;
    case MATRIX_11_A:
        ++j;
    case MATRIX_10_A:
        ++j;
    case MATRIX_9_A:
        ++j;
    case MATRIX_8_A:
        ++j;
    case MATRIX_7_A:
        ++j;
    case MATRIX_6_A:
        ++j;
    case MATRIX_5_A:
        ++j;
    case MATRIX_4_A:
        ++j;
    case MATRIX_3_A:
        ++j;
    case MATRIX_2_A:
        ++j;
    case MATRIX_1_A:
        mtx_vol[j][0]->setValue(value);
        break;
    case MATRIX_18_B:
        ++j;
    case MATRIX_17_B:
        ++j;
    case MATRIX_16_B:
        ++j;
    case MATRIX_15_B:
        ++j;
    case MATRIX_14_B:
        ++j;
    case MATRIX_13_B:
        ++j;
    case MATRIX_12_B:
        ++j;
    case MATRIX_11_B:
        ++j;
    case MATRIX_10_B:
        ++j;
    case MATRIX_9_B:
        ++j;
    case MATRIX_8_B:
        ++j;
    case MATRIX_7_B:
        ++j;
    case MATRIX_6_B:
        ++j;
    case MATRIX_5_B:
        ++j;
    case MATRIX_4_B:
        ++j;
    case MATRIX_3_B:
        ++j;
    case MATRIX_2_B:
        ++j;
    case MATRIX_1_B:
        mtx_vol[j][1]->setValue(value);
        break;
    case MATRIX_18_C:
        ++j;
    case MATRIX_17_C:
        ++j;
    case MATRIX_16_C:
        ++j;
    case MATRIX_15_C:
        ++j;
    case MATRIX_14_C:
        ++j;
    case MATRIX_13_C:
        ++j;
    case MATRIX_12_C:
        ++j;
    case MATRIX_11_C:
        ++j;
    case MATRIX_10_C:
        ++j;
    case MATRIX_9_C:
        ++j;
    case MATRIX_8_C:
        ++j;
    case MATRIX_7_C:
        ++j;
    case MATRIX_6_C:
        ++j;
    case MATRIX_5_C:
        ++j;
    case MATRIX_4_C:
        ++j;
    case MATRIX_3_C:
        ++j;
    case MATRIX_2_C:
        ++j;
    case MATRIX_1_C:
        mtx_vol[j][2]->setValue(value);
        break;
    case MATRIX_18_D:
        ++j;
    case MATRIX_17_D:
        ++j;
    case MATRIX_16_D:
        ++j;
    case MATRIX_15_D:
        ++j;
    case MATRIX_14_D:
        ++j;
    case MATRIX_13_D:
        ++j;
    case MATRIX_12_D:
        ++j;
    case MATRIX_11_D:
        ++j;
    case MATRIX_10_D:
        ++j;
    case MATRIX_9_D:
        ++j;
    case MATRIX_8_D:
        ++j;
    case MATRIX_7_D:
        ++j;
    case MATRIX_6_D:
        ++j;
    case MATRIX_5_D:
        ++j;
    case MATRIX_4_D:
        ++j;
    case MATRIX_3_D:
        ++j;
    case MATRIX_2_D:
        ++j;
    case MATRIX_1_D:
        mtx_vol[j][3]->setValue(value);
        break;
    case MATRIX_18_E:
        ++j;
    case MATRIX_17_E:
        ++j;
    case MATRIX_16_E:
        ++j;
    case MATRIX_15_E:
        ++j;
    case MATRIX_14_E:
        ++j;
    case MATRIX_13_E:
        ++j;
    case MATRIX_12_E:
        ++j;
    case MATRIX_11_E:
        ++j;
    case MATRIX_10_E:
        ++j;
    case MATRIX_9_E:
        ++j;
    case MATRIX_8_E:
        ++j;
    case MATRIX_7_E:
        ++j;
    case MATRIX_6_E:
        ++j;
    case MATRIX_5_E:
        ++j;
    case MATRIX_4_E:
        ++j;
    case MATRIX_3_E:
        ++j;
    case MATRIX_2_E:
        ++j;
    case MATRIX_1_E:
        mtx_vol[j][4]->setValue(value);
        break;
    case MATRIX_18_F:
        ++j;
    case MATRIX_17_F:
        ++j;
    case MATRIX_16_F:
        ++j;
    case MATRIX_15_F:
        ++j;
    case MATRIX_14_F:
        ++j;
    case MATRIX_13_F:
        ++j;
    case MATRIX_12_F:
        ++j;
    case MATRIX_11_F:
        ++j;
    case MATRIX_10_F:
        ++j;
    case MATRIX_9_F:
        ++j;
    case MATRIX_8_F:
        ++j;
    case MATRIX_7_F:
        ++j;
    case MATRIX_6_F:
        ++j;
    case MATRIX_5_F:
        ++j;
    case MATRIX_4_F:
        ++j;
    case MATRIX_3_F:
        ++j;
    case MATRIX_2_F:
        ++j;
    case MATRIX_1_F:
        mtx_vol[j][5]->setValue(value);
        break;
    case MATRIX_18_G:
        ++j;
    case MATRIX_17_G:
        ++j;
    case MATRIX_16_G:
        ++j;
    case MATRIX_15_G:
        ++j;
    case MATRIX_14_G:
        ++j;
    case MATRIX_13_G:
        ++j;
    case MATRIX_12_G:
        ++j;
    case MATRIX_11_G:
        ++j;
    case MATRIX_10_G:
        ++j;
    case MATRIX_9_G:
        ++j;
    case MATRIX_8_G:
        ++j;
    case MATRIX_7_G:
        ++j;
    case MATRIX_6_G:
        ++j;
    case MATRIX_5_G:
        ++j;
    case MATRIX_4_G:
        ++j;
    case MATRIX_3_G:
        ++j;
    case MATRIX_2_G:
        ++j;
    case MATRIX_1_G:
        mtx_vol[j][6]->setValue(value);
        break;
    case MATRIX_18_H:
        ++j;
    case MATRIX_17_H:
        ++j;
    case MATRIX_16_H:
        ++j;
    case MATRIX_15_H:
        ++j;
    case MATRIX_14_H:
        ++j;
    case MATRIX_13_H:
        ++j;
    case MATRIX_12_H:
        ++j;
    case MATRIX_11_H:
        ++j;
    case MATRIX_10_H:
        ++j;
    case MATRIX_9_H:
        ++j;
    case MATRIX_8_H:
        ++j;
    case MATRIX_7_H:
        ++j;
    case MATRIX_6_H:
        ++j;
    case MATRIX_5_H:
        ++j;
    case MATRIX_4_H:
        ++j;
    case MATRIX_3_H:
        ++j;
    case MATRIX_2_H:
        ++j;
    case MATRIX_1_H:
        mtx_vol[j][7]->setValue(value);
        break;
    default:
        fprintf(stderr, "MixSisCtrl::set: invalid alsa ID: %d\n", alsa_id);
    }
}
