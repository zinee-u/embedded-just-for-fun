#ifndef SENSOR_H
#define SENSOR_H
#include <stdbool.h>
#include <stdint.h>

typedef struct { bool ready; } Sensor;

/* 전원 투입 후 Board 배선을 설정. 성공 후 sensor_init 호출. */
bool board_init_sensor_bus(void);

/* 호출자는 Sensor를 {false}로 초기화. NULL 인자는 실패 처리. */
bool sensor_init(Sensor *sensor);
int16_t sensor_decode_x(uint8_t low, uint8_t high);
bool sensor_sample(const Sensor *sensor, uint16_t seq);
#endif
