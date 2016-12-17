#include <cstring>
#include <QtCore>
#include <poll.h>
#include "mixsis.h"
#include "mainwindow.h"
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
    // boot up alsa interface
    snd_ctl_open(&ctl, device, 0);

    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);

    snd_ctl_elem_info_t *info;
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_info_set_numid(info, (int) alsa_numid::USB_SYNC);
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

    // setup volume controls
    int minVol = 0, maxVol = 100;
    // min/max slider values
    for(int k=0;k<18;++k){
        for(int l=0;l<8;++l){
            ctrls->mtx_vol[k][l]->setMinimum((int)minVol);
            ctrls->mtx_vol[k][l]->setMaximum((int)maxVol);
            ctrls->mtx_vol[k][l]->setTracking(trackingp);
        }
        if(k>=6) continue; // only 6 vol_out[k]
        ctrls->vol_out[k]->setMinimum((int)minVol);
        ctrls->vol_out[k]->setMaximum((int)maxVol);
        ctrls->vol_out[k]->setTracking(trackingp);
        if(k>=2) continue; // only 2 vol_master[k]
        ctrls->vol_master[k]->setMinimum((int)minVol);
        ctrls->vol_master[k]->setMaximum((int)maxVol);
        ctrls->vol_master[k]->setTracking(trackingp);
    }
    // get control values from alsa and set widgets accordingly
    for(int i=(int)alsa_numid::MSTR_SWITCH; i<=(int)alsa_numid::INPUT_ROUTE_6; ++i){
        /// this seems roundabout but is necessary due to an alsa bug
        snd_ctl_elem_id_set_numid(id, i);
        snd_ctl_elem_info_set_id(info, id);
        snd_ctl_elem_info(ctl, info);
        snd_ctl_elem_info_get_id(info, id);

        helem = snd_hctl_find_elem(hctl, id);
        snd_hctl_elem_info(helem, info);
        snd_ctl_elem_type_t type = snd_ctl_elem_info_get_type(info);
        snd_ctl_elem_value_set_numid(value, i);
        snd_hctl_elem_read(helem, value);

        int count = snd_ctl_elem_info_get_count(info);
        int val;
        bool isVolume = MixSisCtrl::numidIsVolume((alsa_numid)i);
        for(int idx = 0; idx < count; ++idx){
            if(type == SND_CTL_ELEM_TYPE_INTEGER){
                val = snd_ctl_elem_value_get_integer(value,idx);
                if(isVolume){
                    val = dB_from_volume(val, (alsa_numid)i, ctl);
                }
            }
            else if(type == SND_CTL_ELEM_TYPE_BOOLEAN){
                val = snd_ctl_elem_value_get_boolean(value,idx);
            }
            else if(type == SND_CTL_ELEM_TYPE_ENUMERATED){
                val = snd_ctl_elem_value_get_enumerated(value,idx);
            }
            else{
                fprintf(stderr, "invalid index type, i=%d, idx=%d\n", i, idx);
                continue;
            }
            ctrls->set(i,val,idx);
        }

    }
    // set volume link controls intelligently
    for(int i=0; i<3; ++i){
        ctrls->vol_out_link[i]->setChecked(ctrls->vol_out[2*i]->value() == ctrls->vol_out[2*i+1]->value());
    }
    // hook up callback lambdas
    obj->connect(ctrls->vol_master[0], &QSlider::valueChanged,
            [=](){
        int sixiVol = volume_from_dB(ctrls->vol_master[0]->value(),alsa_numid::MSTR_VOL, ctl);
        this->set(alsa_numid::MSTR_VOL, sixiVol);
        bool blocked = ctrls->vol_master[0]->blockSignals(true);
        ctrls->vol_master[1]->setValue(ctrls->vol_master[0]->value());
        ctrls->vol_master[1]->blockSignals(blocked);
    });
    obj->connect(ctrls->vol_master[1], &QSlider::valueChanged,
            [=](){
        int sixiVol = volume_from_dB(ctrls->vol_master[1]->value(),alsa_numid::MSTR_VOL, ctl);
        this->set(alsa_numid::MSTR_VOL, sixiVol);
        bool blocked = ctrls->vol_master[1]->blockSignals(true);
        ctrls->vol_master[0]->setValue(ctrls->vol_master[1]->value());
        ctrls->vol_master[1]->blockSignals(blocked);
    });
    obj->connect(ctrls->vol_master_mute, &QCheckBox::stateChanged,
            [=](int checkstate){
        this->set(alsa_numid::MSTR_SWITCH, !(checkstate == Qt::Checked), 0);
    });
    int n;
    //  pass by value lambda statements let the if statements work okay inside the lambda
    for(n=0; n<6; ++n){
        // volume out controls 1-6
        obj->connect(ctrls->vol_out[n], &QSlider::valueChanged,
                [=](){
            int volume = ctrls->vol_out[n]->value();
            int which_control = n/2;
            int other_idx = (n&1) ? n-1 : n+1; // if idx = 1, other = 0,etc
            if(ctrls->vol_out_link[which_control]->isChecked()){
                if(ctrls->vol_out[other_idx]->value() != volume){
                    bool old = ctrls->vol_out[n]->blockSignals(true);
                    ctrls->vol_out[other_idx]->setValue(volume);
                    ctrls->vol_out[n]->blockSignals(old);
                }
            }
            alsa_numid control_id;
            if(which_control == 0){
                control_id = alsa_numid::OUT_VOL_12;
            }
            else if(which_control == 1){
                control_id = alsa_numid::OUT_VOL_34;
            }
            else if(which_control == 2){
                control_id = alsa_numid::OUT_VOL_56;
            }
            else{
                fprintf(stderr, "invalid volume out set: %d\nhow did ths happen?\ncrashing...\n", which_control);
                QEvent quitting(QEvent::Quit);
                ((MainWindow*)parent)->event(&quitting);
                // crash
                return;
            }
#ifdef DEBUG
            fprintf(stderr, "volume ctrl %d\n",n);
#endif
            int value = volume_from_dB(volume, control_id, ctl);
            this->set(control_id, value, n&1);

        });
        /// these next two event handlers stop changes in alsa from feeding back with changes in qsismix (by blocking them until slider is released)
        obj->connect(ctrls->vol_out[n], &QSlider::sliderPressed,
                     [=](){
            ((MainWindow*)parent)->setChangeWatcherMask(n/2 | MASK_VOL, true);
        });
        obj->connect(ctrls->vol_out[n], &QSlider::sliderReleased,
                     [=](){
            ((MainWindow*)parent)->setChangeWatcherMask(n/2 | MASK_VOL, false);

        });
        obj->connect(ctrls->vol_out_mute[n], &QCheckBox::stateChanged,
                [=](int checkstate){
            bool value = checkstate ? 1 : 0;
            int which_control = n/2;
            int other_idx = (n&1) ? n-1 : n+1;
#ifdef DEBUG
            fprintf(stderr, "signal: ctl: %d idx: %d val: %d\n", which_control, n&1, value);
#endif
            alsa_numid control_id;
            if(which_control == 0){
                control_id = alsa_numid::OUT_SWITCH_12;
            }
            else if(which_control == 1){
                control_id = alsa_numid::OUT_SWITCH_34;
            }
            else if(which_control == 2){
                control_id = alsa_numid::OUT_SWITCH_56;
            }
            else{
                fprintf(stderr, "invalid mute set: %d\n how did this happen\n", which_control);
                return;
            }
            // this stops feedback here
            ((MainWindow*)parent)->setChangeWatcherMask(n/2 | MASK_MUTE, true);

            if(ctrls->vol_out_link[which_control]->isChecked()){
                if(ctrls->vol_out_mute[other_idx]->isChecked() != value){
                    bool old = ctrls->vol_out_mute[n]->blockSignals(true);
                    ctrls->vol_out_mute[other_idx]->setChecked(value);
                    ctrls->vol_out_mute[n]->blockSignals(old);
                }
            }
            this->set(control_id, !value, n&1);
            // if the other idx is accepting signals, then it didn't call this idx
            if(! ctrls->vol_out_mute[other_idx]->signalsBlocked()){
                ((MainWindow*)parent)->setChangeWatcherMask(which_control | MASK_MUTE, false);
            }
        });
        obj->connect(ctrls->out_src[n], static_cast< void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                [=](int index){
            alsa_numid control_id;
            switch(n){
            case 0:
                control_id = alsa_numid::OUT_1_SRC;
                break;
            case 1:
                control_id = alsa_numid::OUT_2_SRC;
                break;
            case 2:
                control_id = alsa_numid::OUT_3_SRC;
                break;
            case 3:
                control_id = alsa_numid::OUT_4_SRC;
                break;
            case 4:
                control_id = alsa_numid::OUT_5_SRC;
                break;
            case 5:
                control_id = alsa_numid::OUT_6_SRC;
                break;
            default:
                fprintf(stderr, "error: tried to set invalid output src %d\n", n);
                return;
            }
            this->set(control_id, index, 0);
        });
        obj->connect(ctrls->in_src[n], static_cast< void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                [=](int index){
            alsa_numid control_id;
            switch(n){
            case 0:
                control_id = alsa_numid::INPUT_ROUTE_1;
                break;
            case 1:
                control_id = alsa_numid::INPUT_ROUTE_2;
                break;
            case 2:
                control_id = alsa_numid::INPUT_ROUTE_3;
                break;
            case 3:
                control_id = alsa_numid::INPUT_ROUTE_4;
                break;
            case 4:
                control_id = alsa_numid::INPUT_ROUTE_5;
                break;
            case 5:
                control_id = alsa_numid::INPUT_ROUTE_6;
                break;
            default:
                fprintf(stderr, "couldn't set invalid input route %d\n", n);
                return;
            }
            this->set(control_id, index);
        });
    }
    for(int n=0; n<8; n+=2){

        obj->connect(ctrls->in_pad[n], &QRadioButton::toggled,
                     [=](){
            bool value = ctrls->in_pad[n+1]->isChecked();
            alsa_numid control_id;
            switch(n/2){
            case 0:
                control_id = alsa_numid::IN_PAD_1;
                break;
            case 1:
                control_id = alsa_numid::IN_PAD_2;
                break;
            case 2:
                control_id = alsa_numid::IN_PAD_3;
                break;
            case 3:
                control_id = alsa_numid::IN_PAD_4;
                break;
            default:
                fprintf(stderr, "error: tried to toggle in_pad %d\n", n);
                return;
            }
            this->set(control_id, value, 0);
        });
    }
    for(int n=0; n<4; n += 2){
        obj->connect(ctrls->in_imp[n], &QRadioButton::toggled,
                     [=](){
           bool value = ctrls->in_imp[n+1]->isChecked();
           alsa_numid control_id;
           if(n/2 == 0){
               control_id = alsa_numid::IN_IMP_1;
           }
           else if(n/2 == 1){
               control_id = alsa_numid::IN_IMP_2;
           }
           else{
               fprintf(stderr, "error: tried to toggle in_imp %d\n", n);
               return;
           }
           this->set(control_id, value, 0);
        });
    }
    int matrix_index = (int) alsa_numid::MATRIX_ROUTE_1;
    for(n=0;n<18;++n){
        obj->connect(ctrls->mtx_src[n], static_cast< void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                     [=](int index){
            this->set((alsa_numid)matrix_index, index);
        });
        matrix_index += 1;
        for(int i=0; i<8; ++i){
            obj->connect(ctrls->mtx_vol[n][i], &QSlider::valueChanged,
                         [=](){
                int volume = ctrls->mtx_vol[n][i]->value();
                int value = volume_from_dB(volume, (alsa_numid)matrix_index, ctl);
                this->set((alsa_numid)matrix_index, value);
            });
            matrix_index += 1;
        }


    }

    // now, set up the 'clear matrix' buttons
    // they just set the controls; the controls will set the mixer itself
    for(n=0;n<18;++n){
        obj->connect(ctrls->mtx_clear[n], &QPushButton::clicked,
        [=](){
            for(int i=0;i<8;++i){
                ctrls->mtx_vol[n][i]->setValue(0);
            }
            ctrls->mtx_src[n]->setCurrentIndex(0);
        });
        ctrls->mtx_clear[n]->setText(obj->tr("Clear"));
    }
}

MixSis::~MixSis(){

}

// converts to 6i6-internal volume from slider %
// this vaguely represents the way an audio taper pot scales volume
int MixSis::volume_from_dB(int value, alsa_numid controlID, snd_ctl_t *control){
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);

    get_alsa_id(control, id, controlID);

    long mindB;
    snd_ctl_convert_to_dB(control, id, 0, &mindB);
    // algorithm: 0<x<=20 -> mindb<db<=-40, 20<x -> -40<db<=0
    if(value == 0) return 0;
    else if(value <= 20){
        long db = value * (-4000. - mindB)/20. + mindB;
        long volume;
        snd_ctl_convert_from_dB(control, id, db, &volume, 0);
#ifdef DEBUG
        fprintf(stderr, "#1 got six volume %ld from slider %d\n", volume, value);
#endif
        return volume;
    }
    else{
        long db = (value-20) * 4000./80. + -4000.;
        long volume;
        snd_ctl_convert_from_dB(control, id, db, &volume, 0);
#ifdef DEBUG
        fprintf(stderr, "#2 got six volume %ld from slider %d\n", volume, value);
#endif
        return volume;
    }
}

// converts to slider % from 6i6-internal volume
// this isn't 1:1 for some reason and causes problems.
int MixSis::dB_from_volume(int value, alsa_numid controlID, snd_ctl_t *control){
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);

    get_alsa_id(control, id, controlID);

    // algorithm: mindB<dB<-40 -> 0<x<20, -40<dB<0+ -> 20<x<=100
    long mindB;
    snd_ctl_convert_to_dB(control, id, 0, &mindB);
    long dB;
    snd_ctl_convert_to_dB(control, id, value, &dB);
    if(dB<-4000){
        int slider_vol = (dB - mindB) * 20./(-4000. - mindB);
#ifdef DEBUG
        fprintf(stderr, "#1: got slider %d from six vol %d; db=%ld\n", slider_vol, value, dB);
#endif
        return slider_vol;
    }
    else{
        int slider_vol = (dB + 4000.) * 80./4000. + 20;
        if(slider_vol > 100) slider_vol = 100;
#ifdef DEBUG
        fprintf(stderr, "#2: got slider %d from six vol %d\n", slider_vol, value);
#endif
        return slider_vol;
    }
}

void MixSis::set(alsa_numid Nid, int val, int idx){
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_info_alloca(&info);

    get_alsa_id(ctl, id, Nid);
    snd_ctl_elem_id_set_numid(id, Nid);
    snd_ctl_elem_info_set_id(info, id);
    snd_ctl_elem_info(ctl, info);
    snd_ctl_elem_value_t *value;
    snd_ctl_elem_value_alloca(&value);

    snd_ctl_elem_value_set_id(value, id);
    snd_ctl_elem_read(ctl, value);
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
#ifdef DEBUG
        fprintf(stderr,"MixSis::set: wrong type %d numid %d \n",type, Nid);
#endif
    }
    snd_ctl_elem_write(ctl, value);
    return;
}

// gets valid snd_id_t corresponding to alsa_numid
// before calling: id must be allocated, and this->ctl must be open (should be open during lifetime of MixSis)
void MixSis::get_alsa_id(snd_ctl_t *ctl, snd_ctl_elem_id_t*& id, alsa_numid numid){
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_id_set_numid(id, numid);
    snd_ctl_elem_info_set_id(info, id);
    snd_ctl_elem_info(ctl, info);
    snd_ctl_elem_info_get_id(info, id);
}
