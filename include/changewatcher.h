#ifndef CHANGEWATCHER_H
#define CHANGEWATCHER_H

#include "qthread.h"
#include "alsa/asoundlib.h"
#include "mixsisctrl.h"
class ChangeWatcher : public QThread{
Q_OBJECT
public:
    ChangeWatcher(snd_ctl_t *ctl, QObject *parent);
    void run();
    snd_ctl_t *ctl;

    public slots:

    void maskVol(int num, bool mask);

signals:
    void changeVal(int numid, int val, int idx);

private:
    bool isVolBlocked[3];

};

#endif // CHANGEWATCHER_H

