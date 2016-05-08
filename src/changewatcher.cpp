#include <QEvent>
#include "alsa/asoundef.h"
#include "changewatcher.h"
#include "mainwindow.h"
#include "mixsis.h"

ChangeWatcher::ChangeWatcher(snd_ctl_t *ctl, QObject *parent): QThread(parent), ctl(ctl){
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
                fprintf(stderr, "error reading event: %s\n", strerror(err));
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
                exit(-1);
            }
            if(!(mask & SND_CTL_EVENT_MASK_VALUE)){
                continue;
            }
            snd_ctl_elem_value_alloca(&value);
            snd_ctl_elem_id_alloca(&id);
            snd_ctl_elem_info_alloca(&info);
            snd_ctl_event_elem_get_id(event, id);
            snd_ctl_elem_value_set_id(value,id);
            snd_ctl_elem_read(ctl,value);

            idx = snd_ctl_event_elem_get_index(event);
            numid = snd_ctl_event_elem_get_numid(event);

            snd_ctl_elem_info_set_numid(info, numid);
            snd_ctl_elem_info(ctl, info);
            bool isVolume = MixSisCtrl::numidIsVolume( (MixSisCtrl::alsa_numid) numid );
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

            //fprintf(stderr, "numid: %d; val:%d; idx:%d\n", numid, val, idx);
            if(isVolume){
                val = MixSis::dB_from_volume(val, (MixSisCtrl::alsa_numid) numid, ctl);
            }
            emit changeVal(numid, val, idx);
        }
    }
    exit(0);
}

void ChangeWatcher::signalChange(int numid, int val, int idx){
    changeVal(numid, val, idx);
}
