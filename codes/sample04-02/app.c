#include "app.h"
#include "util.h"
#include "horn.h"
#include "timer.h"

const int bumper_sensor = EV3_PORT_1;
const int linemon_sensor = EV3_PORT_3;
const int left_motor = EV3_PORT_A;
const int right_motor = EV3_PORT_C;

const int carrier_sensor = EV3_PORT_2;

int carrier_cargo_is_loaded(void) {
  return ev3_touch_sensor_is_pressed(carrier_sensor);
}

const int walldetector_sensor = EV3_PORT_4;
#define WD_DISTANCE 10
int wd_distance = WD_DISTANCE;

int wall_detector_is_detected(void) {
  return ev3_ultrasonic_sensor_get_distance(walldetector_sensor)
    < wd_distance;
}

int bumper_is_pushed(void) {
  return ev3_touch_sensor_is_pressed(bumper_sensor);
}

#define LM_THRESHOLD 20
int lm_threshold = LM_THRESHOLD;

int linemon_is_online(void) {
  return ev3_color_sensor_get_reflect(linemon_sensor) < lm_threshold;
}

#define DR_POWER 20
int dr_power = DR_POWER;

void driver_turn_left(void) {
  ev3_motor_set_power(EV3_PORT_A, 0);
  ev3_motor_set_power(EV3_PORT_C, dr_power);
}

void driver_turn_right(void) {
  ev3_motor_set_power(EV3_PORT_A, dr_power);
  ev3_motor_set_power(EV3_PORT_C, 0);
}

void driver_stop(void) {
  ev3_motor_stop(left_motor, false);
  ev3_motor_stop(right_motor, false);
}

void tracer_run(void) {
  if(linemon_is_online()) {
    driver_turn_left();
  } else {
    driver_turn_right();
  }
}

void tracer_stop(void) {
  driver_stop();
}

typedef enum {
  P_WAIT_FOR_LOADING, P_TRANSPORTING,
  P_WAIT_FOR_UNLOADING, P_RETURNING, P_ARRIVED
} porter_state;

porter_state p_state = P_WAIT_FOR_LOADING;

int p_entry = true;
timer_t load_timer;  // 積載待ち用タイマー

void porter_transport(void) {
  switch(p_state) {
    case P_WAIT_FOR_LOADING:
      if(p_entry) {
        timer_start(&load_timer, 10000);  // 10秒タイマーを開始
        p_entry = false;
      }
      
      // タイマーが時間切れになったら確認音を鳴らし、タイマーを再スタート
      if(timer_is_timedout(&load_timer)) {
        horn_confirmation();
        timer_start(&load_timer, 10000);
      }

      if(carrier_cargo_is_loaded()) {
        p_state = P_TRANSPORTING;
        p_entry = true;
      }
      break;

    case P_TRANSPORTING:
      if(p_entry) {
        p_entry = false;
      }
      tracer_run();
      if(wall_detector_is_detected()) {
        p_state = P_WAIT_FOR_UNLOADING;
        p_entry = true;
      }
      break;

    case P_WAIT_FOR_UNLOADING:
      if(p_entry) {
        horn_arrived();  // 到着音を鳴らす
        p_entry = false;
      }
      tracer_stop();
      if(!carrier_cargo_is_loaded()) {
        p_state = P_RETURNING;
        p_entry = true;
      }
      break;

    case P_RETURNING:
      if(p_entry) {
        p_entry = false;
      }
      tracer_run();
      if(bumper_is_pushed()) {
        p_state = P_ARRIVED;
        p_entry = true;
      }
      break;

    case P_ARRIVED:
      if(p_entry) {
        horn_arrived();  // 到着音を鳴らす
        tracer_stop();
        p_entry = false;
      }
      break;
  }
}

void main_task(intptr_t unused) {
  init_f("sample04");
  
  ev3_motor_config(left_motor, LARGE_MOTOR);
  ev3_motor_config(right_motor, LARGE_MOTOR);
  
  ev3_sensor_config(bumper_sensor, TOUCH_SENSOR);
  ev3_sensor_config(carrier_sensor, TOUCH_SENSOR);
  ev3_sensor_config(linemon_sensor, COLOR_SENSOR);
  ev3_sensor_config(walldetector_sensor, ULTRASONIC_SENSOR);
  
  /* horn に初期化関数は不要（util/horn.c に初期化処理はありません） */
  timer_initialize(&load_timer);  // タイマーの初期化

  while(!ev3_button_is_pressed(BACK_BUTTON)) {
    porter_transport();
    tslp_tsk(10*1000U);
  }

  ext_tsk();
}