#include <cstring>
#include <QtCore>
#include <poll.h>
#include "mixsis.h"

MixSis::MixSis(MixSisCtrl *ctrls, const char* device, QObject *obj) : controls(ctrls), parent(obj)
{
    int err;
    err = snd_hctl_open(&hctl, device, 0);
    if(err){
        printf("error: unable to access device %s. error %s\n", device, strerror(err));
        exit(err);
    }
    err = snd_hctl_load(hctl);
    if(err){
        printf("error: unable to load mixer hctl. error%d\n", err);
        exit(err);
    }
    // TODO: just use ctl no hctl
    // QUANDARY: is that even possible?
    ctl = snd_hctl_ctl(hctl);

    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);

    snd_ctl_elem_info_t *info;
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_info_set_numid(info,3);
    snd_ctl_elem_info(ctl, info);

    // check whether this is a 6i6
    const char* name = snd_ctl_elem_info_get_name(info);
    if(strcmp("Scarlett 6i6 USB-Sync", name)){
        printf("your card at %s does not appear to be a Scarlett 6i6.\n"
               "looking for string 'Scarlett 6i6 USB-Sync'', got string '%s'\n", device, name);
        exit(1);
    }

    // now that we know it's a scarlett, first, get all the values and set them
    snd_ctl_elem_value_t *value;
    snd_ctl_elem_value_alloca(&value);
    // need hctl elem for great justice
    snd_hctl_elem_t *helem;
    bool trackingp = true;
    // setup dB stuff from card info
    int minVol = 0, maxVol = 100;
    // setup min/max slider values
    for(int k=0;k<18;++k){
        for(int l=0;l<8;++l){
            ctrls->mtx_vol[k][l]->setMinimum((int)minVol);
            ctrls->mtx_vol[k][l]->setMaximum((int)maxVol);
            ctrls->mtx_vol[k][l]->setTracking(trackingp);
        }
        if(k>=6) continue;
        ctrls->vol_out[k]->setMinimum((int)minVol);
        ctrls->vol_out[k]->setMaximum((int)maxVol);
        ctrls->vol_out[k]->setTracking(trackingp);
        if(k>=2) continue;
        ctrls->vol_master[k]->setMinimum((int)minVol);
        ctrls->vol_master[k]->setMaximum((int)maxVol);
        ctrls->vol_master[k]->setTracking(trackingp);
    }
    // get control values from alsa and set widgets accordingly
    for(int i=(int)MixSisCtrl::alsa_numid::MSTR_SWITCH; i<=(int)MixSisCtrl::alsa_numid::INPUT_ROUTE_6; ++i){
        /// this seems roundabout but is necessary due to an alsa bug
        snd_ctl_elem_id_set_numid(id, i);
        snd_ctl_elem_info_set_id(info, id);
        snd_ctl_elem_info(ctl, info);
        snd_ctl_elem_info_get_id(info,id);

        helem = snd_hctl_find_elem(hctl, id);
        snd_hctl_elem_info(helem, info);
        snd_ctl_elem_type_t type = snd_ctl_elem_info_get_type(info);
        snd_ctl_elem_value_set_numid(value, i);
        snd_hctl_elem_read(helem, value);
        int count = snd_ctl_elem_info_get_count(info);
        int val;
        bool isVolume = MixSisCtrl::numidIsVolume((MixSisCtrl::alsa_numid)i);
        for(int idx = 0; idx < count; ++idx){
            if(type == SND_CTL_ELEM_TYPE_INTEGER){
                val = snd_ctl_elem_value_get_integer(value,idx);
                if(isVolume){
                    val = dB_from_volume(val, (MixSisCtrl::alsa_numid)i, ctl);
                }
                ctrls->set(i,val, idx);
            }
            else if(type == SND_CTL_ELEM_TYPE_BOOLEAN){
                val = snd_ctl_elem_value_get_boolean(value,idx);
                ctrls->set(i,val,idx);
            }
            else if(type == SND_CTL_ELEM_TYPE_ENUMERATED){
                val = snd_ctl_elem_value_get_enumerated(value,idx);
                ctrls->set(i,val,idx);
            }
            else{
                fprintf(stderr, "Something went wrong, i=%d, idx=%d\n", i, idx);
            }
        }

    }

    obj->connect(ctrls->vol_master[0], &QSlider::valueChanged,
            [=](){
        int sixiVol = volume_from_dB(ctrls->vol_master[0]->value(),MixSisCtrl::alsa_numid::MSTR_VOL, ctl);
        set(MixSisCtrl::alsa_numid::MSTR_VOL, sixiVol);
        ctrls->vol_master[1]->setValue(ctrls->vol_master[0]->value());
    });
    obj->connect(ctrls->vol_master[1], &QSlider::valueChanged,
            [=](){
        int sixiVol = volume_from_dB(ctrls->vol_master[1]->value(),MixSisCtrl::alsa_numid::MSTR_VOL, ctl);
        set(MixSisCtrl::alsa_numid::MSTR_VOL, ctrls->vol_master[1]->value());
        ctrls->vol_master[0]->setValue(ctrls->vol_master[1]->value());
    });
    obj->connect(ctrls->vol_master_mute, &QCheckBox::stateChanged,
            [=](int checkstate){
        set(MixSisCtrl::alsa_numid::MSTR_SWITCH, ! (checkstate == Qt::Checked), 0);
    });
    int n;
    // todo: the if statements and all should perhaps be outside the lambda? works fine this way, though. if it ain't broke.
    for(n=0; n<6; ++n){
        obj->connect(ctrls->vol_out[n], &QSlider::valueChanged,
                [=](){
            int volume = ctrls->vol_out[n]->value();
            int which_control = n/2;
            int other_idx = (n&1) ? n-1 : n+1; // if idx = 1, other = 0,etc
            if(ctrls->vol_out_link[which_control]->isChecked()){
                if(ctrls->vol_out[other_idx]->value() != volume){
                    ctrls->vol_out[other_idx]->setValue(volume);
                }
            }
            MixSisCtrl::alsa_numid control_id;
            if(which_control == 0){
                control_id = MixSisCtrl::alsa_numid::OUT_VOL_12;
            }
            else if(which_control == 1){
                control_id = MixSisCtrl::alsa_numid::OUT_VOL_34;
            }
            else if(which_control == 2){
                control_id = MixSisCtrl::alsa_numid::OUT_VOL_56;
            }
            else{
                fprintf(stderr, "invalid volume out set: %d\n how did this happen\n", which_control);
                return;
            }
            int value = volume_from_dB(volume, control_id, ctl);
            this->set(control_id, value, n&1);

        });
        obj->connect(ctrls->vol_out_mute[n], &QCheckBox::stateChanged,
                [=](int checkstate){
            bool value = checkstate ? 1 : 0;
            int which_control = n/2;
            int other_idx = (n&1) ? n-1 : n+1;
            if(ctrls->vol_out_link[which_control]->isChecked()){
                if(ctrls->vol_out_mute[other_idx]->isChecked() != value){
                    ctrls->vol_out_mute[other_idx]->setChecked(value);
                }
            }
            MixSisCtrl::alsa_numid control_id;
            if(which_control == 0){
                control_id = MixSisCtrl::alsa_numid::OUT_SWITCH_12;
            }
            else if(which_control == 1){
                control_id = MixSisCtrl::alsa_numid::OUT_SWITCH_34;
            }
            else if(which_control == 2){
                control_id = MixSisCtrl::alsa_numid::OUT_SWITCH_56;
            }
            else{
                fprintf(stderr, "invalid mute set: %d\n how did this happen\n", which_control);
                return;
            }
            this->set(control_id, ! value, n&1);
        });
        obj->connect(ctrls->out_src[n], static_cast< void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                [=](int index){
            MixSisCtrl::alsa_numid control_id;
            switch(n){
            case 0:
                control_id = MixSisCtrl::alsa_numid::OUT_1_SRC;
                break;
            case 1:
                control_id = MixSisCtrl::alsa_numid::OUT_2_SRC;
                break;
            case 2:
                control_id = MixSisCtrl::alsa_numid::OUT_3_SRC;
                break;
            case 3:
                control_id = MixSisCtrl::alsa_numid::OUT_4_SRC;
                break;
            case 4:
                control_id = MixSisCtrl::alsa_numid::OUT_5_SRC;
                break;
            case 5:
                control_id = MixSisCtrl::alsa_numid::OUT_6_SRC;
                break;
            default:
                fprintf(stderr, "error: tried to set invalid output src %d\n", n);
                return;
            }
            this->set(control_id, index, 0);
        });
        obj->connect(ctrls->in_src[n], static_cast< void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                [=](int index){
            MixSisCtrl::alsa_numid control_id;
            switch(n){
            case 0:
                control_id = MixSisCtrl::alsa_numid::INPUT_ROUTE_1;
                break;
            case 1:
                control_id = MixSisCtrl::alsa_numid::INPUT_ROUTE_2;
                break;
            case 2:
                control_id = MixSisCtrl::alsa_numid::INPUT_ROUTE_3;
                break;
            case 3:
                control_id = MixSisCtrl::alsa_numid::INPUT_ROUTE_4;
                break;
            case 4:
                control_id = MixSisCtrl::alsa_numid::INPUT_ROUTE_5;
                break;
            case 5:
                control_id = MixSisCtrl::alsa_numid::INPUT_ROUTE_6;
                break;
            default:
                fprintf(stderr, "couldn't set invalid input route %d\n", n);
                return;
            }
            this->set(control_id, index);
        });
        if(n >= 4) continue;
        obj->connect(ctrls->in_pad[n], &QRadioButton::toggled,
                     [=](){
            bool value = ctrls->in_pad[2*n+1]->isChecked();
            MixSisCtrl::alsa_numid control_id;
            switch(n){
            case 0:
                control_id = MixSisCtrl::alsa_numid::IN_PAD_1;
                break;
            case 1:
                control_id = MixSisCtrl::alsa_numid::IN_PAD_2;
                break;
            case 2:
                control_id = MixSisCtrl::alsa_numid::IN_PAD_3;
                break;
            case 3:
                control_id = MixSisCtrl::alsa_numid::IN_PAD_4;
                break;
            default:
                fprintf(stderr, "error: tried to toggle in_pad %d\n", n);
                return;
            }
            this->set(control_id, value, 0);
        });
        if(n >= 2) continue;
        obj->connect(ctrls->in_imp[n], &QRadioButton::toggled,
                     [=](){
           bool value = ctrls->in_imp[2*n+1]->isChecked();
           MixSisCtrl::alsa_numid control_id;
           if(n == 0){
               control_id = MixSisCtrl::alsa_numid::IN_IMP_1;
           }
           else if(n == 1){
               control_id = MixSisCtrl::alsa_numid::IN_IMP_2;
           }
           else{
               fprintf(stderr, "error: tried to toggle in_imp %d\n", n);
               return;
           }
           this->set(control_id, value, 0);
        });
    }
    int matrix_index = (int) MixSisCtrl::alsa_numid::MATRIX_ROUTE_1;
    for(n=0;n<18;++n){
        obj->connect(ctrls->mtx_src[n], static_cast< void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                [=](int index){
            this->set((MixSisCtrl::alsa_numid)matrix_index, index);
        });
        matrix_index += 1;
        for(int i=0; i<8; ++i){
            obj->connect(ctrls->mtx_vol[n][i], &QSlider::valueChanged,
            [=](){
                int volume = ctrls->mtx_vol[n][i]->value();
                int value = volume_from_dB(volume, (MixSisCtrl::alsa_numid)matrix_index, ctl);
                this->set((MixSisCtrl::alsa_numid)matrix_index, value);
            });
            matrix_index += 1;
        }


    }
}

MixSis::~MixSis(){

}

// converts to 6i6-internal volume from slider %
// this vaguely represents the way an audio taper pot scales volume
// this also requires the snd_ctl_t to be valid, but this shouldn't be an issue at all since qsismix won't start without alsa connection and exits if alsa connection is lost
int MixSis::volume_from_dB(int value, MixSisCtrl::alsa_numid controlID, snd_ctl_t *control){
    snd_ctl_elem_id_t *dBID;
    snd_ctl_elem_id_alloca(&dBID);
    snd_ctl_elem_info_t *dBInfo;
    snd_ctl_elem_info_alloca(&dBInfo);
    snd_ctl_elem_id_set_numid(dBID, controlID);
    snd_ctl_elem_info_set_id(dBInfo, dBID);
    snd_ctl_elem_info(control, dBInfo);
    snd_ctl_elem_info_get_id(dBInfo, dBID);
    long mindB;
    snd_ctl_convert_to_dB(control, dBID, 0, &mindB);
    // algorithm: 0<x<=20 -> mindb<db<=-40, 20<x -> -40<db<=0
    if(value == 0) return 0;
    else if(value <= 20){
        long db = value * (-4000. - mindB)/20. + mindB;
        long volume;
        snd_ctl_convert_from_dB(control, dBID, db, &volume, 0);
        //fprintf(stderr, "got six volume %ld from slider %d\n", volume, value);
        return volume;
    }
    else{
        long db = (value-20) * 4000./80. + -4000.;
        long volume;
        snd_ctl_convert_from_dB(control, dBID, db, &volume, 0);
        //fprintf(stderr, "got six volume %ld from slider %d\n", volume, value);
        return volume;
    }
}

// converts to slider % from 6i6-internal volume
int MixSis::dB_from_volume(int value, MixSisCtrl::alsa_numid controlID, snd_ctl_t *control){
    snd_ctl_elem_id_t *dBID;
    snd_ctl_elem_id_alloca(&dBID);
    snd_ctl_elem_info_t *dBInfo;
    snd_ctl_elem_info_alloca(&dBInfo);
    snd_ctl_elem_id_set_numid(dBID, controlID);
    snd_ctl_elem_info_set_id(dBInfo, dBID);
    snd_ctl_elem_info(control, dBInfo);
    snd_ctl_elem_info_get_id(dBInfo, dBID);
    // algorithm: mindB<dB<-40 -> 0<x<20, -40<dB<0+, 20<x<=100
    long mindB;
    snd_ctl_convert_to_dB(control, dBID, 0, &mindB);
    long dB;
    snd_ctl_convert_to_dB(control, dBID, value, &dB);
    if(dB<-4000){
        int slider_vol = (dB - mindB) * 20./(-4000. - mindB);
        //fprintf(stderr, "#1: got slider %d from six vol %d; db=%ld\n", slider_vol, value, dB);
        return slider_vol;
    }
    else{
        int slider_vol = (dB + 4000.) * 80./4000. + 20;
        if(slider_vol > 100) slider_vol = 100;
        //fprintf(stderr, "#2: got slider %d from six vol %d\n", slider_vol, value);
        return slider_vol;
    }
}

void MixSis::set(MixSisCtrl::alsa_numid Nid, int val, int idx){
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_value_t *value;
    snd_ctl_elem_value_alloca(&value);

    snd_ctl_elem_id_set_numid(id, Nid);
    snd_ctl_elem_info_set_id(info, id);
    snd_ctl_elem_info(ctl, info);
    snd_ctl_elem_info_get_id(info,id);

    snd_hctl_elem_t *helem = snd_hctl_find_elem(hctl,id);
    snd_hctl_elem_read(helem,value);
    snd_hctl_elem_info(helem,info);
    snd_ctl_elem_type_t type = snd_ctl_elem_info_get_type(info);
    if(type == SND_CTL_ELEM_TYPE_BOOLEAN){
        snd_ctl_elem_value_set_boolean(value,idx, val);
    }
    else if(type == SND_CTL_ELEM_TYPE_INTEGER){
        snd_ctl_elem_value_set_integer(value,idx,val);
    }
    else if(type == SND_CTL_ELEM_TYPE_ENUMERATED){
        snd_ctl_elem_value_set_enumerated(value,idx,val);
    }
    else{
        fprintf(stderr,"something has gone wrong with numid %d in MixSis::set\n",Nid);
    }
    snd_hctl_elem_write(helem,value);
    return;
}
