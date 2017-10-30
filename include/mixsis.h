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
    MixSisCtrl *controls;
    snd_hctl_t *hctl;
    QObject *parent;
    //long mindB, maxdB;
    snd_mixer_t *mixer;

    void set_vol_mute(int which, bool);
public:

    MixSis(MixSisCtrl* ctrls, const char *device, QObject *obj);
    ~MixSis();

    snd_ctl_t *ctl;
    void set(alsa_numid, int, int = 0);

    static int dB_from_volume(int value, alsa_numid controlID, snd_ctl_t *control);
    static int volume_from_dB(int value, alsa_numid controlID, snd_ctl_t *control);
    static void get_alsa_id(snd_ctl_t *ctl, snd_ctl_elem_id_t*& id, alsa_numid numid);

};

#endif // MIX6I6_H
