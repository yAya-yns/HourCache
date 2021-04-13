 # include <stdio.h>
// # include "drive/gpio.h"
// # include "sdkconfig.h"
 # include <time.h>
 # include <stdlib.h>
 # include <assert.h>

int WAKE_PIN = 17;
int GYRO_PIN = -1;
int REF_TIME = -1;
int Q_MAX_SIZE = 100; // TODO: calculate precise value that our memory can handle

typedef enum state_t {
    Device_sleep = -1,
    Study = 0,
    Rest = 1,
    Sleep = 2,
    Eat = 3,
    Phone = 4,
    Device_undefined = 5,
    Play = 6,
} state_t;

typedef struct period_t {
    state_t state;
    float start;
    float end;
} period_t;

typedef struct qnode_t {
    period_t * period;
    struct qnode_t * next;
} qnode_t;

typedef struct queue_t {
    int count;
    qnode_t * start;
    qnode_t * end;
} queue_t;

// state queue
queue_t * squeue = NULL;
period_t * prev_period = NULL;

// just for safety
void interrupts_off(){
    detachInterrupt(WAKE_PIN);
}

// interrupt service routine
void isr(){
    // does interrupts shut down when in ISR?

    // recording previous period
    if(prev_period->state != Device_sleep){
        prev_period->end = millis(); // NOTE: arduino only
        append_queue(squeue, prev_period);
    }

    // new period begins
//    prev_period->state = get_state(GYRO_PIN);
    prev_period->start = millis();  // NOTE: arduino only

    // sending
    while(squeue->count > 10){ // TODO: 10 is arbitrary
        period_t * pd = pop_queue(squeue);
//        send_state(pd);   // sending state info through wifi
    }
}

void interrupts_on(){
    attachInterrupt(digitalPinToInterrupt(WAKE_PIN), isr, RISING);  // NOTE: arduino only
//    attachInterrupt(WAKE_PIN, isr, RISING);  // NOTE: ESP only
}

void append_queue(queue_t * q, period_t * pd){
  assert(q != NULL);
  assert(pd != NULL);
  assert(q->count <= Q_MAX_SIZE);

  qnode_t * node = (qnode_t *)malloc(sizeof(qnode_t));
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

period_t * pop_queue(queue_t * q){
  assert(q != NULL);
  assert(q->count != 0);

  qnode_t * temp = q->start;
  q->start = q->start->next;
  q->count -= 1;

  period_t * ret = temp->period;
  free(temp);
  if(q->count == 0){
    q->end = NULL;
  }
  return ret;
}

// gets the state given gyroscope pin
state get_state(int gyroscope_pin);

// send data through wifi
void send_state(period_t * pd);


void setup() {
    Serial.begin(115200);
//    time(&REF_TIME); // TODO: this is a placeholder function, will need to think about how to keep track of time. 
// using internal clock on ESP32?
    REF_TIME = millis(); // NOTE: arduino only
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html#rtc-clock-source
    // system timers accuracy vary based on whether in sleep mode and temperatures, another option is to use wifi to send system 
    // time from computer, which will guarantee accuracy
    //  ^ sounds good
    pinMode(WAKE_PIN, INPUT_PULLUP);
    pinMode(GYRO_PIN, INPUT_PULLUP); // TODO: gyroscope probably needs more than 1 analogue pin, fix later

    assert(squeue == NULL);
    squeue = (queue_t *)malloc(sizeof(queue_t));
    assert(squeue != NULL);
    squeue->count = 0;
    squeue->start = NULL;
    squeue->end = NULL;

    assert(prev_period == NULL);
    prev_period = (period_t *)malloc(sizeof(period_t));
    assert(prev_period);
    prev_period->state = Device_sleep;
    prev_period->start = REF_TIME;

    interrupts_on();
//    set_timer_interrupt();    // TODO: placeholder function, device should wake up every X minutes, hook up to timer ISR
}

// all code will be running in ISR
void loop() {
  
}
