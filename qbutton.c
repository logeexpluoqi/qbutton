/**
 * @ Author: luoqi
 * @ Create Time: 2024-03-29 17:20
 * @ Modified by: luoqi
 * @ Modified time: 2025-02-10 11:22
 * @ Description:
 */

#include "qbutton.h"

#define QBUTTON_EVENT_CALLBACK(action, keyval, local_err)   \
    do {                                                    \
        if (button->callback[action] != 0) {                \
            int ret = button->callback[action](keyval);     \
            local_err = ret;                                \
        }                                                   \
    } while (0)

int qbutton_init(QButton *button, QButtonPressDownKeyVal keyval, uint8_t debounce_tick, uint16_t long_tick, uint8_t short_tick, int (*button_read)(void))
{
    button->isactive      = 0;
    button->debounce      = 0;
    button->ticks         = 0;
    button->repeat        = 0;
    button->press_keyval  = keyval;
    button->button_read   = button_read;
    button->debounce_tick = debounce_tick;
    button->long_tick     = long_tick;
    button->short_tick    = short_tick;
    if (button->short_tick > button->long_tick) {
        button->short_tick = button->long_tick;
    }
    button->state = QBUTTON_ACTION_NONE;
    return 0;
}

int qbutton_events_attach(QButton *button, QButtonAction action, int (*callback)(int))
{
    if (button->callback[action] == 0) {
        button->callback[action] = callback;
        return 0;
    } else {
        return -1;
    }
}

int qbutton_events_detach(QButton *button, QButtonAction action)
{
    if(button->callback[action] != 0) {
        button->callback[action] = 0;
        return 0;
    } else {
        return -1;
    }
}

int qbutton_exec(QButton *button)
{
    int err = 0;
    int keyval = button->button_read();

    if((keyval == button->press_keyval) && !button->isactive) {
        if(button->debounce++ > button->debounce_tick) {
            button->state   = QBUTTON_ACTION_PRESS_DOWN;
            button->isactive = 1;
            button->debounce = 0;
        }
    } else {
        button->debounce = 0;
    }

    switch(button->state) {
    case QBUTTON_ACTION_NONE:
        button->isactive = 0;
        button->repeat   = 0;
        break;
    case QBUTTON_ACTION_PRESS_DOWN:
        if(keyval == button->press_keyval) {
            QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_PRESS_DOWN, keyval, err);
            if(button->ticks++ > button->long_tick) {
                button->state = QBUTTON_ACTION_PRESS_LONG;
            }
        } else {
            button->state = QBUTTON_ACTION_PRESS_UP;
        }
        break;
    case QBUTTON_ACTION_PRESS_UP:
        if(button->islongpress) {
            QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_PRESS_UP, keyval, err);
            button->state = QBUTTON_ACTION_NONE;
            button->islongpress = 0;
            button->ticks = 0;
        } else {
            if(button->callback[QBUTTON_ACTION_PRESS_REPEAT] == 0 &&
               button->callback[QBUTTON_ACTION_SINGLE_CLICK] == 0 &&
               button->callback[QBUTTON_ACTION_DOUBLE_CLICK] == 0 &&
               button->callback[QBUTTON_ACTION_TRIPLE_CLICK] == 0) {
                QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_PRESS_UP, keyval, err);
                button->state = QBUTTON_ACTION_NONE;
                button->ticks = 0;
            } else {
                button->state = QBUTTON_ACTION_PRESS_REPEAT;
            }
        }
        break;
    case QBUTTON_ACTION_PRESS_REPEAT:
        if(button->ticks++ < button->short_tick) {
            if(button->isactive) {
                button->repeat++;
            }
        } else {
            if(button->repeat == 1) {
                button->state = QBUTTON_ACTION_SINGLE_CLICK;
            } else if(button->repeat == 2) {
                if(button->callback[QBUTTON_ACTION_DOUBLE_CLICK] == 0) {
                    button->state = QBUTTON_ACTION_SINGLE_CLICK;
                } else {
                    button->state = QBUTTON_ACTION_DOUBLE_CLICK;
                }
            } else if(button->repeat == 3) {
                if(button->callback[QBUTTON_ACTION_TRIPLE_CLICK] == 0 &&
                   button->callback[QBUTTON_ACTION_DOUBLE_CLICK] == 0) {
                    button->state = QBUTTON_ACTION_SINGLE_CLICK;
                } else {
                    button->state = QBUTTON_ACTION_TRIPLE_CLICK;
                }
            } else {
                QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_PRESS_REPEAT, keyval, err);
                button->state = QBUTTON_ACTION_NONE;
                button->ticks = 0;
                button->repeat = 0;
            }
        }
        button->isactive = 0;
        break;
    case QBUTTON_ACTION_SINGLE_CLICK:
        QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_SINGLE_CLICK, keyval, err);
        button->state = QBUTTON_ACTION_NONE;
        button->ticks = 0;
        break;
    case QBUTTON_ACTION_DOUBLE_CLICK:
        QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_DOUBLE_CLICK, keyval, err);
        button->state = QBUTTON_ACTION_NONE;
        button->ticks = 0;
        break;
    case QBUTTON_ACTION_TRIPLE_CLICK:
        QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_TRIPLE_CLICK, keyval, err);
        button->state = QBUTTON_ACTION_NONE;
        button->ticks = 0;
        break;
    case QBUTTON_ACTION_PRESS_LONG:
        QBUTTON_EVENT_CALLBACK(QBUTTON_ACTION_PRESS_LONG, keyval, err);
        button->state = QBUTTON_ACTION_NONE;
        button->islongpress = 1;
        button->ticks = 0;
        break;
    default:
        return -1;
    }
    return err;
}
