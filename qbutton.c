/**
 * @ Author: luoqi
 * @ Create Time: 2024-03-29 17:20
 * @ Modified by: luoqi
 * @ Modified time: 2025-04-25 11:00
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

int qbutton_init(QButton *button, QButtonPressDownKeyVal kv, uint8_t debounce, uint16_t long_tick, uint8_t short_tick, int (*kv_read)(void))
{
    if(!button || !kv_read) {
        return -1;
    }
    button->isactive = 0;
    button->dticks = 0;
    button->ticks = 0;
    button->repeat = 0;
    button->press_kv = kv;
    button->kv_read = kv_read;
    button->debounce = debounce;
    button->long_tick = long_tick;
    button->short_tick = short_tick;
    button->click_timeout = 0;
    if(button->short_tick > button->long_tick) {
        button->short_tick = button->long_tick;
    }
    button->state = QBUTTON_ACTION_NONE;
    return 0;
}

int qbutton_events_attach(QButton *button, QButtonAction action, int (*callback)(int kv))
{
    if(!button || !callback) {
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
    int kv = button->kv_read();

    if(kv == button->press_kv) {
        if(button->dticks < button->debounce) {
            button->dticks++;
        } else if(!button->isactive) {
            button->state = QBUTTON_ACTION_PRESS_DOWN;
            button->isactive = 1;
            button->dticks = 0;
        } 

    } else {
        button->dticks = 0;
        button->isactive = 0;
    }

    switch(button->state) {
    case QBUTTON_ACTION_NONE:
        button->isactive = 0;
        button->repeat = 0;
        break;

    case QBUTTON_ACTION_PRESS_DOWN:
        if(kv == button->press_kv) {
            QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_PRESS_DOWN, kv, err);
            if(button->ticks++ > button->long_tick) {
                QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_PRESS_LONG, kv, err);
                button->state = QBUTTON_ACTION_PRESS_LONG_HOLD;
            }
        } else {
            if(button->ticks < button->long_tick) {
                button->state = QBUTTON_ACTION_WAIT_MULTICLICK;
            }
        }
        break;

    case QBUTTON_ACTION_PRESS_LONG_HOLD:
        if(kv != button->press_kv) {
            QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_PRESS_UP, kv, err);
            button->state = QBUTTON_ACTION_NONE;
            button->ticks = 0;
        }
        break;

    case QBUTTON_ACTION_WAIT_MULTICLICK:
        button->click_timeout++;
        if(kv == button->press_kv) {
            button->repeat++;
            button->state = QBUTTON_ACTION_PRESS_DOWN;
            button->click_timeout = 0;
            button->isactive = 1;
        } else if(button->click_timeout > button->short_tick) {
            if(button->repeat == 0) {
                if(button->callback[QBUTTON_ACTION_SINGLE_CLICK]) {
                    QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_SINGLE_CLICK, kv, err);
                } else {
                    QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_PRESS_UP, kv, err);
                }
            } else if(button->repeat == 1) {
                if(button->callback[QBUTTON_ACTION_DOUBLE_CLICK]) {
                    QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_DOUBLE_CLICK, kv, err);
                } else {
                    QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_SINGLE_CLICK, kv, err);
                }
            } else if(button->repeat == 2) {
                if(button->callback[QBUTTON_ACTION_TRIPLE_CLICK]) {
                    QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_TRIPLE_CLICK, kv, err);
                } else if(button->callback[QBUTTON_ACTION_DOUBLE_CLICK]) {
                    QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_DOUBLE_CLICK, kv, err);
                } else {
                    QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_SINGLE_CLICK, kv, err);
                }
            } else {
                QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_PRESS_REPEAT, kv, err);
            }
            button->state = QBUTTON_ACTION_NONE;
            button->ticks = 0;
            button->repeat = 0;
            button->click_timeout = 0;
        }
        break;

    default:
        return -1;
    }
    return err;
}
