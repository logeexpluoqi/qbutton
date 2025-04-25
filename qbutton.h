/**
 * @ Author: luoqi
 * @ Create Time: 2024-03-29 17:20
 * @ Modified by: luoqi
 * @ Modified time: 2025-04-25 11:00
 * @ Description:
 */

#ifndef _QBUTTON_H
#define _QBUTTON_H

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
 * @brief QButtonPressDownKeyVal defines the key value states for button press down.
 */
typedef enum {
    QBUTTON_PRESS_DOWN_KEYVAL_LOW = 0x00,
    QBUTTON_PRESS_DOWN_KEYVAL_HIGH = 0x01
} QButtonPressDownKeyVal;

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
    uint8_t   press_kv;
    uint16_t  ticks;
    uint8_t   debounce;
    uint8_t   state;
    uint8_t   isactive : 1;
    uint8_t   islongpress : 1;
    uint8_t   repeat : 6;
    uint8_t   dticks;
    uint16_t  long_tick;        // long press time threshold
    uint8_t   short_tick;       // repeat press time threshold
    uint8_t   click_timeout;    // multi-click click timeout

    int (*kv_read)(void);

    QButtonEvent callback[7];
} QButton;

/**
 * @brief Initializes a QButton instance.
 * @param button Pointer to the QButton instance.
 * @param kv Initial key value state.
 * @param debounce Debounce time in ticks.
 * @param long_tick Long press time threshold in ticks.
 * @param short_tick Repeat press time threshold in ticks.
 * @param kv_read Function pointer to read the key value.
 * @return 0 on success, non-zero on failure.
 */
int qbutton_init(QButton *button, QButtonPressDownKeyVal kv, uint8_t debounce, uint16_t long_tick, uint8_t short_tick, int (*kv_read)(void));

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
