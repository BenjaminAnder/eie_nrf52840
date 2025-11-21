


#include <zephyr/kernel.h>
#include <zephyr/smf.h>

#include <stdlib.h>

#include "BTN.h"
#include "LED.h"
#include "message_state_machine.h"


static void character_state_entry(void* o);
static enum smf_state_result character_state_run(void* o);
static void menu_state_entry(void* o);
static enum smf_state_result menu_state_run(void* o);
static void save_state_entry(void* o);
static enum smf_state_result save_state_run(void* o);
static void standby_state_entry(void* o);
static enum smf_state_result standby_state_run(void* o);


enum message_machine_states {
    CHARACTER_STATE,
    MENU_STATE,
    SAVE_STATE,
    STANDBY_STATE
}

typedef struct {
    struct smf_ctx ctx;

    uint8_t ascii;
    uint8_t * string;
    uint16_t length;
    uint8_t standby;
    uint64_t time_s;
} message_state_object_t;


static const struct smf_state message_states[] = {
    [CHARACTER_STATE] = SMF_CREATE_STATE(character_state_entry, character_state_run, NULL, NULL, NULL),
    [MENU_STATE] = SMF_CREATE_STATE(menu_state_entry, menu_state_run, NULL, NULL, NULL)
    [SAVE_STATE] = SMF_CREATE_STATE(save_state_entry, save_state_run, NULL, NULL, NULL),
    [STANDBY_STATE] = SMF_CREATE_STATE(standby_state_entry, standby_state_run, NULL, NULL, NULL)
};

static message_state_object_t message_state_object;

void state_machine_init() {
    message_state_object.ascii = 0; 
    message_state_object.string = calloc(1,sizeof(uint8_t));
    message_state_object.length = 0;
    if (message_state_object.string == NULL) {
        printk("Error allocating memory");
        exit(1);
    }
    message_state_object.length = 0;
    message_state_object.standby = NULL;

    smf_set_initial(SMF_CTX(&message_state_object), &message_states[CHARACTER_STATE]);
}


int state_machine_run() {
    return smf_run_state(SMF_CTX(&message_state_object));
}

static void character_state_entry(void* o) {
    LED_blink(LED3, LED_1HZ);
}

static enum smf_state_result character_state_run(void* o) {
    if (BTN_check_clear_pressed(BTN0)) {
        // add 0 bit to ascii
        message_state_object.ascii = message_state_object.ascii << 1;
    } else if (BTN_check_clear_pressed(BTN1)) {
        // add 1 bit to ascii
        message_state_object.ascii = (message_state_object.ascii << 1) | 1;
    } else if (BTN_check_clear_pressed(BTN2)) {
        // reset ascii
        message_state_object.ascii = 0;
    } else if (BTN_check_clear_pressed(BTN3)) {
        // save ascii to string, reset ascii, change state to save
        message_state_object.string = realloc(message_state_object.string, (message_state_object.length + 2)*typeof(uint8_t));
        if (message_state_object.string == NULL) {
            printk("Error allocating memory");
            exit(1);
        }
        message_state_object.string[++message_state_object.length] = message_state_object.ascii;
        message_state_object.string[message_state_object.length+1] = '\0';
        message_state_object.ascii = 0;
        smf_set_state(SMF_CTX(&message_state_object), &message_states[MENU_STATE]);
    }

    return SMF_EVENT_HANDLED;
}

static void menu_state_entry(void* o) {
    LED_blink(LED3, LED_4HZ);
}

static enum smf_state_result menu_state_run(void* o) {
    if (BTN_check_clear_pressed(BTN0) || BTN_check_clear_pressed(BTN1)) {
        smf_set_state(SMF_CTX(&message_state_object), &message_states[CHARACTER_STATE]);
    } else if (BTN_check_clear_pressed(BTN2)) {
        smf_set_state(SMF_CTX(&message_state_object), &message_states[SAVE_STATE]);
    } else if (BTN_check_clear_pressed(BTN3)) {
        smf_set_state(SMF_CTX(&message_state_object), &message_states[CHARACTER_STATE]);
    }

    return SMF_EVENT_HANDLED;
}

static void save_state_entry(void* o) {
    LED_blink(LED3, LED_16HZ);
}

static enum smf_state_result save_state_run(void* o) {
    if (BTN_check_clear_pressed(BTN2)) {
        //delete string and go back to character state
        message_state_object.string = realloc(message_state_object.string, sizeof(uint8_t));
        if (message_state_object.string == NULL) {
            printk("Error allocating memory");
            exit(1);
        }
        message_state_object.string[0] = '\0';
        smf_set_state(SMF_CTX(&message_state_object), &message_states[CHARACTER_STATE]);
    } else if (BTN_check_clear_pressed(BTN3)) {
        //send string to serial monitor
        printk("%s\n", message_state_object.string);
    }

    return SMF_EVENT_HANDLED;
}

static void standby_state_entry(void* o) {
    LED_blink(LED3, LED_1HZ);
}

static enum smf_state_result standby_state_run(void* o) {
    if (BTN_check_clear_pressed(BTN0)||
        BTN_check_clear_pressed(BTN1)||
        BTN_check_clear_pressed(BTN2)||
        BTN_check_clear_pressed(BTN3)) {

        smf_set_state(SMF_CTX(&message_state_object), &message_states[message_state_object.standby]);
    }

    return SMF_EVENT_HANDLED;
}

int check_enter_standby () {
    message_state_object.time_s = k_uptime_get();
    while(BTN_is_pressed(BTN0)) {}
    if (k_uptime_get() - message_state_object.time_s > 0) {
        
    }
    return 0;
}