/**
 * @ Author: luoqi
 * @ Create Time: 2024-03-29 17:20
 * @ Modified by: luoqi
 * @ Modified time: 2025-05-18 23:32
 * @ Description:
 */

#include "qbutton.h"

#define QBUTTON_EVENT_CALLBACK(action, kv, local_err)   \
    do {                                                \
        if (button->callback[action] != 0) {            \
            int ret = button->callback[action](kv);     \
            local_err = ret;                            \
        }                                               \
    } while (0)

int qbutton_init(QButton *button, QButtonPressedLevel lvl, uint8_t debounce, uint16_t long_tm, uint8_t click_tmo, int (*btn_read)(void))
{
    if(!button || !btn_read) {
        return -1;
    }
    button->pressed = 0;
    button->dbnc = 0;
    button->hold = 0;
    button->clicks = 0;
    button->act_lvl = lvl;
    button->btn_read = btn_read;
    button->debounce = debounce;
    button->long_tm = long_tm;
    button->click_tmo = click_tmo;
    button->tmo = 0;
    for(int i = 0; i < 7; i++) {
        button->callback[i] = 0;
    }
    if(button->click_tmo > button->long_tm) {
        button->click_tmo = button->long_tm;
    }
    button->state = QBUTTON_ACTION_NONE;
    return 0;
}

int qbutton_events_attach(QButton *button, QButtonAction action, int (*callback)(int val))
{
    if(!button || !callback) {
        return -1;
    }
    if(action >= QBUTTON_ACTION_NONE) {
        return -1;
    }
    if(button->callback[action] == 0) {
        button->callback[action] = callback;
        return 0;
    } else {
        return -1;
    }
}

int qbutton_events_detach(QButton *button, QButtonAction action)
{
    if(!button) {
        return -1;
    }
    if(action >= QBUTTON_ACTION_NONE) {
        return -1;
    }
    if(button->callback[action] != 0) {
        button->callback[action] = 0;
        return 0;
    } else {
        return -1;
    }
}

int qbutton_exec(QButton *button)
{
    if(!button) {
        return -1;
    }
    int err = 0;
    int val = button->btn_read();

    if(val == button->act_lvl) {
        if(button->dbnc < button->debounce) {
            button->dbnc++;
        } else if(!button->pressed) {
            button->state = QBUTTON_ACTION_PRESS_DOWN;
            button->pressed = 1;
            button->dbnc = 0;
        } 

    } else {
        button->dbnc = 0;
        button->pressed = 0;
    }

    switch(button->state) {
    case QBUTTON_ACTION_NONE:
        button->pressed = 0;
        button->clicks = 0;
        break;

    case QBUTTON_ACTION_PRESS_DOWN:
        if(val == button->act_lvl) {
            QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_PRESS_DOWN, val, err);
            if(button->hold++ > button->long_tm) {
                QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_PRESS_LONG, val, err);
                button->state = QBUTTON_ACTION_PRESS_LONG_HOLD;
            }
        } else {
            if(button->hold < button->long_tm) {
                button->state = QBUTTON_ACTION_WAIT_MULTICLICK;
            } else {
                /* hold >= long_tm: released at the boundary
                 * after hold++ pushed past long_tm but before
                 * the long-press transition could fire */
                QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_PRESS_UP, val, err);
                button->state = QBUTTON_ACTION_NONE;
                button->hold = 0;
            }
        }
        break;

    case QBUTTON_ACTION_PRESS_LONG_HOLD:
        if(val != button->act_lvl) {
            QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_PRESS_UP, val, err);
            button->state = QBUTTON_ACTION_NONE;
            button->hold = 0;
        }
        break;

    case QBUTTON_ACTION_WAIT_MULTICLICK:
        button->tmo++;
        if(val == button->act_lvl) {
            button->clicks++;
            button->state = QBUTTON_ACTION_PRESS_DOWN;
            button->hold = 0;
            button->tmo = 0;
            button->pressed = 1;
        } else if(button->tmo > button->click_tmo) {
            if(button->clicks == 0) {
                if(button->callback[QBUTTON_ACTION_SINGLE_CLICK]) {
                    QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_SINGLE_CLICK, val, err);
                } else {
                    QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_PRESS_UP, val, err);
                }
            } else if(button->clicks == 1) {
                if(button->callback[QBUTTON_ACTION_DOUBLE_CLICK]) {
                    QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_DOUBLE_CLICK, val, err);
                } else {
                    QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_SINGLE_CLICK, val, err);
                }
            } else if(button->clicks == 2) {
                if(button->callback[QBUTTON_ACTION_TRIPLE_CLICK]) {
                    QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_TRIPLE_CLICK, val, err);
                } else if(button->callback[QBUTTON_ACTION_DOUBLE_CLICK]) {
                    QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_DOUBLE_CLICK, val, err);
                } else {
                    QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_SINGLE_CLICK, val, err);
                }
            } else {
                QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_PRESS_REPEAT, val, err);
            }
            button->state = QBUTTON_ACTION_NONE;
            button->hold = 0;
            button->clicks = 0;
            button->tmo = 0;
        }
        break;

    default:
        return -1;
    }
    return err;
}
