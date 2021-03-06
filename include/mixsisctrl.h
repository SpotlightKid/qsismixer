#ifndef MIXSISCTRL
#define MIXSISCTRL

#include <QSlider>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QDataStream>

#include <alsa/version.h>

// where alsa >= 1.1.2, the numid's start with USB_SYNC=5. where alsa < 1.1.2, they start with USB_SYNC=3. Either way, they sequentially increase in the same order.
// note that as a result, if you upgrade alsa from a version < 1.1.2 to a newer version, you must recompile qsismix so that it will be compatible.

int constexpr kUsbSync() {
    return (SND_LIB_VERSION >= 0x010102) ? 5 : 3;
}

enum alsa_numid {
        USB_SYNC = kUsbSync(), MSTR_SWITCH, MSTR_VOL, OUT_SWITCH_12, OUT_VOL_12, OUT_1_SRC, OUT_2_SRC,
        OUT_SWITCH_34, OUT_VOL_34, OUT_3_SRC, OUT_4_SRC, OUT_SWITCH_56, OUT_VOL_56,
        OUT_5_SRC, OUT_6_SRC, IN_IMP_1, IN_PAD_1, IN_IMP_2, IN_PAD_2, IN_PAD_3, IN_PAD_4,
        MATRIX_ROUTE_1, MATRIX_1_A, MATRIX_1_B, MATRIX_1_C, MATRIX_1_D, MATRIX_1_E, MATRIX_1_F, MATRIX_1_G, MATRIX_1_H,
        MATRIX_ROUTE_2, MATRIX_2_A, MATRIX_2_B, MATRIX_2_C, MATRIX_2_D, MATRIX_2_E, MATRIX_2_F, MATRIX_2_G, MATRIX_2_H,
        MATRIX_ROUTE_3, MATRIX_3_A, MATRIX_3_B, MATRIX_3_C, MATRIX_3_D, MATRIX_3_E, MATRIX_3_F, MATRIX_3_G, MATRIX_3_H,
        MATRIX_ROUTE_4, MATRIX_4_A, MATRIX_4_B, MATRIX_4_C, MATRIX_4_D, MATRIX_4_E, MATRIX_4_F, MATRIX_4_G, MATRIX_4_H,
        MATRIX_ROUTE_5, MATRIX_5_A, MATRIX_5_B, MATRIX_5_C, MATRIX_5_D, MATRIX_5_E, MATRIX_5_F, MATRIX_5_G, MATRIX_5_H,
        MATRIX_ROUTE_6, MATRIX_6_A, MATRIX_6_B, MATRIX_6_C, MATRIX_6_D, MATRIX_6_E, MATRIX_6_F, MATRIX_6_G, MATRIX_6_H,
        MATRIX_ROUTE_7, MATRIX_7_A, MATRIX_7_B, MATRIX_7_C, MATRIX_7_D, MATRIX_7_E, MATRIX_7_F, MATRIX_7_G, MATRIX_7_H,
        MATRIX_ROUTE_8, MATRIX_8_A, MATRIX_8_B, MATRIX_8_C, MATRIX_8_D, MATRIX_8_E, MATRIX_8_F, MATRIX_8_G, MATRIX_8_H,
        MATRIX_ROUTE_9, MATRIX_9_A, MATRIX_9_B, MATRIX_9_C, MATRIX_9_D, MATRIX_9_E, MATRIX_9_F, MATRIX_9_G, MATRIX_9_H,
        MATRIX_ROUTE_10, MATRIX_10_A, MATRIX_10_B, MATRIX_10_C, MATRIX_10_D, MATRIX_10_E, MATRIX_10_F, MATRIX_10_G, MATRIX_10_H,
        MATRIX_ROUTE_11, MATRIX_11_A, MATRIX_11_B, MATRIX_11_C, MATRIX_11_D, MATRIX_11_E, MATRIX_11_F, MATRIX_11_G, MATRIX_11_H,
        MATRIX_ROUTE_12, MATRIX_12_A, MATRIX_12_B, MATRIX_12_C, MATRIX_12_D, MATRIX_12_E, MATRIX_12_F, MATRIX_12_G, MATRIX_12_H,
        MATRIX_ROUTE_13, MATRIX_13_A, MATRIX_13_B, MATRIX_13_C, MATRIX_13_D, MATRIX_13_E, MATRIX_13_F, MATRIX_13_G, MATRIX_13_H,
        MATRIX_ROUTE_14, MATRIX_14_A, MATRIX_14_B, MATRIX_14_C, MATRIX_14_D, MATRIX_14_E, MATRIX_14_F, MATRIX_14_G, MATRIX_14_H,
        MATRIX_ROUTE_15, MATRIX_15_A, MATRIX_15_B, MATRIX_15_C, MATRIX_15_D, MATRIX_15_E, MATRIX_15_F, MATRIX_15_G, MATRIX_15_H,
        MATRIX_ROUTE_16, MATRIX_16_A, MATRIX_16_B, MATRIX_16_C, MATRIX_16_D, MATRIX_16_E, MATRIX_16_F, MATRIX_16_G, MATRIX_16_H,
        MATRIX_ROUTE_17, MATRIX_17_A, MATRIX_17_B, MATRIX_17_C, MATRIX_17_D, MATRIX_17_E, MATRIX_17_F, MATRIX_17_G, MATRIX_17_H,
        MATRIX_ROUTE_18, MATRIX_18_A, MATRIX_18_B, MATRIX_18_C, MATRIX_18_D, MATRIX_18_E, MATRIX_18_F, MATRIX_18_G, MATRIX_18_H,
        INPUT_ROUTE_1, INPUT_ROUTE_2, INPUT_ROUTE_3, INPUT_ROUTE_4, INPUT_ROUTE_5, INPUT_ROUTE_6

};

struct MixSisCtrl
{

    static bool numidIsVolume(alsa_numid test);

    QSlider *vol_master[2];
    QCheckBox *vol_master_mute;
    QCheckBox *vol_master_link;
    QSlider *vol_out[6];
    QCheckBox *vol_out_mute[6];
    QCheckBox *vol_out_link[3];
    QComboBox *out_src[6];
    QComboBox *in_src[6];
    QRadioButton *in_imp[4];
    QRadioButton *in_pad[8];
    QComboBox *mtx_src[18];
    QSlider *mtx_vol[18][8];
    QPushButton *mtx_clear[18];


 //   int save_to_dialog(QWidget *context);
 //   int load_from_dialog(QWidget *context);

    /// gets control values from file
    int load_from(const QString &filename);

    /// dumps control values to file
    int save_to(const QString &filename);

    /// sets the qt control corresponding to alsa_id,idx to a certain value
    void set(int alsa_id, int value, int idx = 0);
};

#endif // MIXSISCTRL

