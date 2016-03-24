#ifndef MIX6I6_H
#define MIX6I6_H

#include <QSlider>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>

#include <alsa/asoundlib.h>

#include "mixsisctrl.h"
#include "changewatcher.h"

class MixSis
{
public:

    MixSis(MixSisCtrl* ctrls, const char *device, QObject *obj);
    ~MixSis();
    MixSisCtrl *controls;
    int value_from_volume(int);
    int volume_from_value(int);
    snd_hctl_t *hctl;
    QObject *parent;
        private:
    snd_ctl_t *ctl;
    void set(MixSisCtrl::alsa_numid, int, int = 0);
    //void set_vol(alsa_numid, int, int = 0);
    void set_vol_mute(int which, bool);
    long mindB, maxdB;

    snd_mixer_t *mixer;
    ChangeWatcher *watch;

};

#endif // MIX6I6_H
