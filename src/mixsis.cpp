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
        printf("this does not appear to be a Scarlett 6i6.\n"
               "looking for string 'Scarlett 6i6 USB-Sync'', got string '%s'\n", name);
        exit(1);
    }

    // now that we know it's a scarlett, first, get all the values and set them
    snd_ctl_elem_value_t *value;
    snd_ctl_elem_value_alloca(&value);
    // setup db volume stuff, they all look the same
    // need hctl elem for great justice
    snd_hctl_elem_t *helem;
    mindB = 82, maxdB = 128;
    bool trackingp = true;
    // setup min/max slider values
    for(int k=0;k<18;++k){
        for(int l=0;l<8;++l){
            ctrls->mtx_vol[k][l]->setMinimum((int)mindB);
            ctrls->mtx_vol[k][l]->setMaximum((int)maxdB);
            ctrls->mtx_vol[k][l]->setTracking(trackingp);
        }
        if(k>=6) continue;
        ctrls->vol_out[k]->setMinimum((int)mindB);
        ctrls->vol_out[k]->setMaximum((int)maxdB);
        ctrls->vol_out[k]->setTracking(trackingp);
        if(k>=2) continue;
        ctrls->vol_master[k]->setMinimum((int)mindB);
        ctrls->vol_master[k]->setMaximum((int)maxdB);
        ctrls->vol_master[k]->setTracking(trackingp);
    }
    // get control values from alsa and set widgets accordingly
    // TODO: this should be its own function, maybe even callback from alsa to update control changes
    for(int i=(int)MixSisCtrl::alsa_numid::MSTR_SWITCH; i<=(int)MixSisCtrl::alsa_numid::INPUT_ROUTE_6; ++i){
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
        for(int idx = 0; idx < count; ++idx){
            if(type == SND_CTL_ELEM_TYPE_INTEGER){
                val = snd_ctl_elem_value_get_integer(value,idx);
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
        set(MixSisCtrl::alsa_numid::MSTR_VOL, ctrls->vol_master[0]->value());
        ctrls->vol_master[1]->setValue(ctrls->vol_master[0]->value());
    });
    obj->connect(ctrls->vol_master[1], &QSlider::valueChanged,
            [=](){
        set(MixSisCtrl::alsa_numid::MSTR_VOL, ctrls->vol_master[1]->value());
        ctrls->vol_master[0]->setValue(ctrls->vol_master[1]->value());
    });
    obj->connect(ctrls->vol_master_mute, &QCheckBox::stateChanged,
            [=](int checkstate){
        set(MixSisCtrl::alsa_numid::MSTR_SWITCH, ! (checkstate == Qt::Checked), 0);
    });
    int n;
    for(n=0; n<6; ++n){
        obj->connect(ctrls->vol_out[n], &QSlider::valueChanged,
                [=](){
            int volume = ctrls->vol_out[n]->value();
            int snpt = n/2;
            int other = (n&1) ? n-1 : n+1;
            if(ctrls->vol_out_link[snpt]->isChecked()){
                if(ctrls->vol_out[other]->value() != volume){
                    ctrls->vol_out[other]->setValue(volume);
                }
            }
            MixSisCtrl::alsa_numid ctl;
            if(snpt == 0){
                ctl = MixSisCtrl::alsa_numid::OUT_VOL_12;
            }
            else if(snpt == 1){
                ctl = MixSisCtrl::alsa_numid::OUT_VOL_34;
            }
            else if(snpt == 2){
                ctl = MixSisCtrl::alsa_numid::OUT_VOL_56;
            }
            else{
                fprintf(stderr, "invalid volume out set %d how did this happen\n", snpt);
                return;
            }
            this->set(ctl, volume, n&1);

        });
        obj->connect(ctrls->vol_out_mute[n], &QCheckBox::stateChanged,
                [=](int checkstate){
            bool value = checkstate ? 1 : 0;
            int snpt = n/2;
            int other = (n&1) ? n-1 : n+1;
            if(ctrls->vol_out_link[snpt]->isChecked()){
                if(ctrls->vol_out_mute[other]->isChecked() != value){
                    ctrls->vol_out_mute[other]->setChecked(value);
                }
            }
            MixSisCtrl::alsa_numid ctl;
            if(snpt == 0){
                ctl = MixSisCtrl::alsa_numid::OUT_SWITCH_12;
            }
            else if(snpt == 1){
                ctl = MixSisCtrl::alsa_numid::OUT_SWITCH_34;
            }
            else if(snpt == 2){
                ctl = MixSisCtrl::alsa_numid::OUT_SWITCH_56;
            }
            else{
                fprintf(stderr, "invalid volume out set %d how did this happen\n", snpt);
                return;
            }
            this->set(ctl, ! value, n&1);
        });
        obj->connect(ctrls->out_src[n], static_cast< void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                [=](int index){
            MixSisCtrl::alsa_numid ctl;
            switch(n){
            case 0:
                ctl = MixSisCtrl::alsa_numid::OUT_1_SRC;
                break;
            case 1:
                ctl = MixSisCtrl::alsa_numid::OUT_2_SRC;
                break;
            case 2:
                ctl = MixSisCtrl::alsa_numid::OUT_3_SRC;
                break;
            case 3:
                ctl = MixSisCtrl::alsa_numid::OUT_4_SRC;
                break;
            case 4:
                ctl = MixSisCtrl::alsa_numid::OUT_5_SRC;
                break;
            case 5:
                ctl = MixSisCtrl::alsa_numid::OUT_6_SRC;
                break;
            default:
                fprintf(stderr, "error: tried to set invalid output src %d\n", n);
                return;
            }
            this->set(ctl, index, 0);
        });
        obj->connect(ctrls->in_src[n], static_cast< void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                [=](int index){
            MixSisCtrl::alsa_numid ctl;
            switch(n){
            case 0:
                ctl = MixSisCtrl::alsa_numid::INPUT_ROUTE_1;
                break;
            case 1:
                ctl = MixSisCtrl::alsa_numid::INPUT_ROUTE_2;
                break;
            case 2:
                ctl = MixSisCtrl::alsa_numid::INPUT_ROUTE_3;
                break;
            case 3:
                ctl = MixSisCtrl::alsa_numid::INPUT_ROUTE_4;
                break;
            case 4:
                ctl = MixSisCtrl::alsa_numid::INPUT_ROUTE_5;
                break;
            case 5:
                ctl = MixSisCtrl::alsa_numid::INPUT_ROUTE_6;
                break;
            default:
                fprintf(stderr, "couldn't set invalid input route %d\n", n);
                return;
            }
            this->set(ctl, index);
        });
        if(n >= 4) continue;
        obj->connect(ctrls->in_pad[n], &QRadioButton::toggled,
                     [=](){
            bool value = ctrls->in_pad[n]->isChecked();
            MixSisCtrl::alsa_numid ctl;
            switch(n){
            case 0:
                ctl = MixSisCtrl::alsa_numid::IN_PAD_1;
                break;
            case 1:
                ctl = MixSisCtrl::alsa_numid::IN_PAD_2;
                break;
            case 2:
                ctl = MixSisCtrl::alsa_numid::IN_PAD_3;
                break;
            case 3:
                ctl = MixSisCtrl::alsa_numid::IN_PAD_4;
                break;
            default:
                fprintf(stderr, "error: tried to toggle in_pad %d\n", n);
                return;
            }
            this->set(ctl, value, 0);
        });
        if(n >= 2) continue;
        obj->connect(ctrls->in_imp[n], &QRadioButton::toggled,
                     [=](){
           bool value = ctrls->in_imp[n]->isChecked();
           MixSisCtrl::alsa_numid ctl;
           if(n == 0){
               ctl = MixSisCtrl::alsa_numid::IN_IMP_1;
           }
           else if(n == 1){
               ctl = MixSisCtrl::alsa_numid::IN_IMP_2;
           }
           else{
               fprintf(stderr, "error: tried to toggle in_imp %d\n", n);
               return;
           }
           this->set(ctl, value, 0);
        });
    }
    int MTX_ROUTE = (int) MixSisCtrl::alsa_numid::MATRIX_ROUTE_1;
    for(n=0;n<18;++n){
        obj->connect(ctrls->mtx_src[n], static_cast< void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                [=](int index){
            this->set((MixSisCtrl::alsa_numid)MTX_ROUTE, index);
        });
        MTX_ROUTE += 1;
        for(int i=0; i<8; ++i){
            obj->connect(ctrls->mtx_vol[n][i], &QSlider::valueChanged,
            [=](){
                int volume = ctrls->mtx_vol[n][i]->value();
                int value = value_from_volume(volume);
                this->set((MixSisCtrl::alsa_numid)MTX_ROUTE, value);
            });
            MTX_ROUTE += 1;
        }


    }
    watch = new ChangeWatcher(ctl, controls, parent);
    watch->start();
}

MixSis::~MixSis(){

}

int MixSis::value_from_volume(int volume){
    if(volume <= mindB) return 0;
    return volume;
}

int MixSis::volume_from_value(int value){
    return value;
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
