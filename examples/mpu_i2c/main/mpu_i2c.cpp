// =========================================================================
// Released under the MIT License
// Copyright 2017-2018 Natanael Josue Rabello. All rights reserved.
// For the license information refer to LICENSE file in root directory.
// =========================================================================

/**
 * @file mpu_i2c.cpp
 * Example on how to setup MPU through I2C for basic usage.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "I2Cbus.hpp"
#include "MPU.hpp"
#include "mpu/math.hpp"
#include "mpu/types.hpp"

static const char* TAG = "example";

static constexpr gpio_num_t SDA = GPIO_NUM_4;
static constexpr gpio_num_t SCL = GPIO_NUM_5;
static constexpr uint32_t CLOCK_SPEED = 400000;  // range from 100 KHz ~ 400Hz

extern "C" void app_main() {
    printf("$ MPU Driver Example: MPU-I2C\n");
    fflush(stdout);

    // Initialize I2C on port 0 using I2Cbus interface
    i2c0.begin(SDA, SCL, CLOCK_SPEED);

    // Or directly with esp-idf API
    /*
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA;
    conf.scl_io_num = SCL;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = CLOCK_SPEED;
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0));
    */

    MPU_t MPU;  // create a default MPU object
    MPU.setBus(i2c0);  // set bus port, not really needed since default is i2c0
    MPU.setAddr(mpud::MPU_I2CADDRESS_AD0_LOW);  // set address, default is AD0_LOW

    // Great! Let's verify the communication
    // (this also check if the connected MPU supports the implementation of chip selected in the component menu)
    while (esp_err_t err = MPU.testConnection()) {
        ESP_LOGE(TAG, "Failed to connect to the MPU, error=%#X", err);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "MPU connection successful!");

    // Initialize
    ESP_ERROR_CHECK(MPU.initialize());  // initialize the chip and set initial configurations
    ESP_ERROR_CHECK(MPU.setAuxI2CBypass(false));//取消旁路接入
    ESP_ERROR_CHECK(MPU.setAuxI2CEnabled(true));//允许mpu9250主模式读取寄存器
    // Setup with your configurations
    // ESP_ERROR_CHECK(MPU.setSampleRate(50));  // set sample rate to 50 Hz
    // ESP_ERROR_CHECK(MPU.setGyroFullScale(mpud::GYRO_FS_500DPS));
    // ESP_ERROR_CHECK(MPU.setAccelFullScale(mpud::ACCEL_FS_4G));

    // Reading sensor data
    printf("Reading sensor data:\n");
    mpud::sensors_t sensors;
    while (true) {
        MPU.sensors(&sensors, 0);
        printf("A: [%6d,%6d,%6d] (G) \t\n", sensors.accel.x,sensors.accel.y,sensors.accel.z);
        printf("G: [%6d,%6d,%6d] (G) \t\n", sensors.gyro.x,sensors.gyro.y,sensors.gyro.z);
        printf("M: [%6d,%6d,%6d] (G) \t\n", sensors.mag.x,sensors.mag.y,sensors.mag.z);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
