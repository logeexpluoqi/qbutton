/**
 * Author: luoqi
 * Created Date: 2025-12-23 16:08:59
 * Last Modified: 2026-05-26 11:41:19
 * Modified By: luoqi at <**@****>
 * Copyright (c) 2026 <*****>
 * Description: 
 */

#ifndef _QBUTTON_H_
#define _QBUTTON_H_

#include <stdint.h>

typedef enum {
    QBUTTON_ACTION_PRESS_DOWN = 0x00,
    QBUTTON_ACTION_PRESS_UP,
    QBUTTON_ACTION_PRESS_REPEAT,
    QBUTTON_ACTION_SINGLE_CLICK,
    QBUTTON_ACTION_DOUBLE_CLICK,
    QBUTTON_ACTION_TRIPLE_CLICK,
    QBUTTON_ACTION_PRESS_LONG,
    
    /* process state, do not use for user */
    QBUTTON_ACTION_NONE,
    QBUTTON_ACTION_PRESS_LONG_HOLD,
    QBUTTON_ACTION_WAIT_MULTICLICK,
} QButtonAction;

/**
 * @brief QButtonPressedLevel defines the key value states for button pressed.
 */
typedef enum {
    QBUTTON_PRESSED_LVL_LOW = 0x00,
    QBUTTON_PRESSED_LVL_HIGH = 0x01
} QButtonPressedLevel;

/**
 * @brief QButtonEvent is a function pointer type for button event callbacks.
 * @param kv The key value associated with the event.
 * @return An integer indicating the result of the callback.
 */
typedef int (*QButtonEvent)(int);

/**
 * @brief QButton represents the structure for a button with its properties and event callbacks.
 */
typedef struct {
    uint8_t   act_lvl;         // active level: pin value that means "pressed"
    uint16_t  hold;            // hold duration counter
    uint8_t   debounce;        // debounce threshold
    uint8_t   state;           // current FSM state
    uint8_t   pressed : 1;     // button is in pressed state
    uint8_t   clicks : 7;      // multi-click counter (0=1st,1=2nd,2=3rd,3+=repeat)
    uint8_t   dbnc;            // debounce tick counter
    uint16_t  long_tm;         // long-press threshold (ticks)
    uint8_t   click_tmo;       // multi-click timeout threshold (ticks)
    uint8_t   tmo;             // timeout tick counter

    int (*btn_read)(void);     // pin read function

    QButtonEvent callback[7];  // event callbacks
} QButton;

/**
 * @brief Initializes a QButton instance.
 * @param button Pointer to the QButton instance.
 * @param lvl Active level: pin value that means "pressed" (LOW or HIGH).
 * @param debounce Debounce threshold in ticks.
 * @param long_tm Long-press threshold in ticks.
 * @param click_tmo Multi-click timeout threshold in ticks.
 * @param btn_read Function pointer to read the pin value.
 * @return 0 on success, non-zero on failure.
 */
int qbutton_init(QButton *button, QButtonPressedLevel lvl, uint8_t debounce, uint16_t long_tm, uint8_t click_tmo, int (*btn_read)(void));

/**
 * @brief Attaches a callback function to a specific button action.
 * @param button Pointer to the QButton instance.
 * @param action The button action to attach the callback to.
 * @param callback Function pointer to the callback.
 * @return 0 on success, non-zero on failure.
 */
int qbutton_events_attach(QButton *button, QButtonAction action, int (*callback)(int kv));

/**
 * @brief Detaches a callback function from a specific button action.
 * @param button Pointer to the QButton instance.
 * @param action The button action to detach the callback from.
 * @return 0 on success, non-zero on failure.
 */
int qbutton_events_detach(QButton *button, QButtonAction action);
 
/**
 * @brief Executes the button logic, processing its state and triggering callbacks.
 * @param button Pointer to the QButton instance.
 * @return 0 on success, non-zero on failure.
 */
int qbutton_exec(QButton *button);

#endif
