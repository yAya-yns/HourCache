// # include <stdio.h>
// # include "drive/gpio.h"
// # include "sdkconfig.h"
// # include <time.h>
// # include <stdlib.h>

WAKE_PIN = -1;
GYRO_PIN = -1;
REF_TIME = -1;
Q_MAX_SIZE = 100; // TODO: calculate precise value that our memory can handle

typedef enum state {
    Device_sleep = -1,
    Study = 0,
    Rest = 1,
    Sleep = 2,
    Eat = 3,
    Phone = 4,
    Device_undefined = 5,
    Play = 6,
} state;

typedef struct period {
    state state;
    float start;
    float end;
} period;

typedef struct qnode {
    period * period;
    struct qnode * next;
} qnode;

typedef struct queue {
    int count;
    qnode * start;
    qnode * end;
} queue;

// state queue
queue * squeue = NULL;
period * prev_period = NULL;

// just for safety
void interrupts_off(){
    detachInterrupt(WAKE_PIN);
}

void interrupts_on(){
    attachInterrupt(WAKE_PIN, ISR, RISING);
}

void append_queue(queue * q, period * pd){
	assert(q != NULL);
	assert(pd != NULL);
    assert(q->count <= Q_MAX_SIZE);

	qnode * node = (qnode *)malloc(sizeof(qnode));
	assert(node != NULL);
	node->period = pd;
	node->next = NULL;

	if(q->count == 0){
		q->start = node;
		q->end = node;
	}else{
		q->end->next = node;
		q->end = node;
	}
	q->count += 1;
}

period* pop_queue(queue * q){
	assert(q != NULL);
	assert(q->count != 0);

	qnode * temp = q->start;
	q->start = q->start->next;
	q->count -= 1;

	period * ret = temp->period;
	free(temp);
	if(q->count == 0){
		q->end = NULL;
	}
	return ret;
}

// gets the state given gyroscope pin
state get_state(int gyroscope_pin);

// send data through wifi
void send_state(period * pd);

// interrupt service routine
void ISR(){
    // does interrupts shut down when in ISR?

    // recording previous period
    if(prev_period->state != Device_sleep){
        prev_period->end = time();
        append_queue(squeue, prev_period);
    }

    // new period begins
    prev_period->state = get_state(GYRO_PIN);
    prev_period->start = time();

    // sending
    while(squeue->count > 10){ // TODO: 10 is arbitrary
        period * pd = pop_queue(squeue);
        send_state(pd);
    }
}


void setup() {
    Serial.begin(115200);
    REF_TIME = time(); // TODO: this is a placeholder function, will need to think about how to keep track of time. 
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html#rtc-clock-source
    // system timers accuracy vary based on whether in sleep mode and temperatures, another option is to use wifi to send system 
    // time from computer, which will guarantee accuracy
    pinMode(WAKE_PIN, INPUT_PULLUP);
    pinMode(GYRO_PIN, INPUT_PULLUP); // TODO: gyroscope probably needs more than 1 analogue pin, fix later

    assert(squeue == NULL);
    squeue = (queue *)malloc(sizeof(queue));
    assert(squeue != NULL);
    squeue->count = 0;
    squeue->start = NULL;
    squeue->end = NULL;

    assert(prev_period == NULL);
    prev_period = (period *)malloc(sizeof(period));
    assert(prev_period);
    prev_period->state = Device_sleep;
    prev_period->start = REF_TIME;

    interrupts_on();
    set_timer_interrupt();    // TODO: placeholder function, device should wake up every X minutes, hook up to timer ISR
}

// all code will be running in ISR
void loop() {
  
}

