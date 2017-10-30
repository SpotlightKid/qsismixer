#include <QEvent>
#include "alsa/asoundef.h"
#include "changewatcher.h"
#include "mainwindow.h"
#include "mixsis.h"

constexpr bool volHasSecondIdx(alsa_numid numid){
    return (numid == alsa_numid::OUT_VOL_12) || (numid == alsa_numid::OUT_VOL_34) || (numid == alsa_numid::OUT_VOL_56);
}

ChangeWatcher::ChangeWatcher(snd_ctl_t *ctl, QObject *parent): QThread(parent), ctl(ctl), isVolMasked {0}, isMuteMasked {0}, isMatrixVolMasked{0} {

}

void ChangeWatcher::run(){
    snd_ctl_subscribe_events(ctl, 1);
    snd_ctl_event_t *event;
    unsigned short revents;
    unsigned int mask;
    struct pollfd fd;
    int err;
    int numid, idx, val;
    snd_ctl_elem_value_t *value;
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_value_alloca(&value);
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_info_alloca(&info);
    while(!this->isInterruptionRequested()){
        snd_ctl_poll_descriptors(ctl, &fd, 1);
        err = poll(&fd, 1, -1);
        if (err <= 0){
            continue;
        }
        snd_ctl_poll_descriptors_revents(ctl, &fd, 1, &revents);
        if(revents & POLLIN){
            snd_ctl_event_alloca(&event);
            err = snd_ctl_read(ctl, event);
            if(err < 0){
                fprintf(stderr, "error reading event from audio backend: %s\n", strerror(err));
                continue;
            }
            if(snd_ctl_event_get_type(event) != SND_CTL_EVENT_ELEM){
                continue;
            }
            mask = snd_ctl_event_elem_get_mask(event);
            if(mask == SND_CTL_EVENT_MASK_REMOVE){
                fprintf(stdout,"signal: alsa device removed, goodbye\n");
                QEvent quitting(QEvent::Quit);
                ((MainWindow*)parent())->event(&quitting);
                break;
            }
            if(!(mask & SND_CTL_EVENT_MASK_VALUE)){
                continue;
            }
            numid = snd_ctl_event_elem_get_numid(event);
            if(isNumidMasked((alsa_numid)numid)){
                continue;
            }
            snd_ctl_event_elem_get_id(event, id);
            snd_ctl_elem_value_set_id(value, id);
            snd_ctl_elem_read(ctl, value);
            idx = snd_ctl_event_elem_get_index(event); // this call always seems to return 0 but is the "right" way to implement the api
            snd_ctl_elem_info_set_numid(info, numid);
            snd_ctl_elem_info(ctl, info);

            switch(snd_ctl_elem_info_get_type(info)){
            case SND_CTL_ELEM_TYPE_BOOLEAN:
                val = snd_ctl_elem_value_get_boolean(value, idx);
                break;
            case SND_CTL_ELEM_TYPE_ENUMERATED:
                val = snd_ctl_elem_value_get_enumerated(value, idx);
                break;
            default:
                fprintf(stderr, "changewatcher: invalid snd ctl elem type %s\n", snd_ctl_elem_type_name(snd_ctl_elem_info_get_type(info)));
            case SND_CTL_ELEM_TYPE_INTEGER:
                val = snd_ctl_elem_value_get_integer(value, idx);
            }
#ifdef DEBUG
            fprintf(stderr, "change = numid: %d; val:%d; idx:%d\n", numid, val, idx);
#endif
            bool isVolume = MixSisCtrl::numidIsVolume( (alsa_numid) numid );
            if(isVolume){
                val = MixSis::dB_from_volume(val, (alsa_numid) numid, ctl);
            }
            emit changeVal(numid, val, 0);
            if(isVolume && volHasSecondIdx((alsa_numid)numid)){
                val = snd_ctl_elem_value_get_integer(value, 1);
                val = MixSis::dB_from_volume(val, (alsa_numid) numid, ctl);
                emit changeVal(numid, val, 1);
            }
            else if(numid == alsa_numid::OUT_SWITCH_12 || numid == alsa_numid::OUT_SWITCH_34 || numid == alsa_numid::OUT_SWITCH_56){
                val = snd_ctl_elem_value_get_boolean(value, 1);
                emit changeVal(numid, val, 1);
            }
        }
    }
}

void ChangeWatcher::setMask(int num, int mask){
#ifdef DEBUG
    fprintf(stderr, "set mask %x to %d\n", num, mask);
#endif
    if(num & MASK_MTX_VOL){
        num = num & ~MASK_MTX_VOL;
        int mtx = num/18;
        int vol = num%18;
        if(mask){
            // set mask bit
            isMatrixVolMasked[mtx] |= 1 << vol;
        }
        else{
            // clear mask bit
            isMatrixVolMasked[mtx] &= ~(1 << vol);
        }
    }
    else if(num & MASK_VOL){
        num = num & ~MASK_VOL;
        if((num < 3)&&(num >=0)){
            isVolMasked[num] = mask;
        }
#ifdef DEBUG
        else{
            fprintf(stderr, "ChangeWatcher::maskVol: invalid mask num %d\n", num);
        }
#endif
    }
    else if(num & MASK_MUTE){
        num = num & ~MASK_MUTE;
        if((num < 3) && (num >= 0)){
            isMuteMasked[num] = mask;
        }
#ifdef DEBUG
        else fprintf(stderr, "ChangeWatcher::setMask: invalid mask num %d\n", num);
#endif
    }
}

bool ChangeWatcher::isNumidMasked(alsa_numid numid){
    return (isVolMasked[0] && (numid == alsa_numid::OUT_VOL_12)) || (isVolMasked[1] && (numid == alsa_numid::OUT_VOL_34)) || (isVolMasked[2] && (numid == alsa_numid::OUT_VOL_56)) ||
            (isMuteMasked[0] && (numid == alsa_numid::OUT_SWITCH_12)) || (isMuteMasked[1] && (numid == alsa_numid::OUT_SWITCH_34)) || (isMuteMasked[2] && (numid == alsa_numid::OUT_SWITCH_56));
}
