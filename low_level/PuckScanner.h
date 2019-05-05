#ifndef PUCK_SCANNER_h
#define PUCK_SCANNER_h

#include <ToF_sensor.h>
#include "Config.h"
#include "CommunicationServer.h"

#define SCAN_SENSOR_MIN             (15)      // mm
#define SCAN_SENSOR_MAX             (200)     // mm
#define SCAN_RESOLUTION             (101)     // resolution spaciale selon l'axe Y
#define SCAN_LEFT_SENSOR_POSITION	(-43.83)  // mm
#define SCAN_RIGHT_SENSOR_POSITION	(43.83)   // mm
#define SCAN_EDGE_MIN_HEIGHT        (15.0)    // mm
#define SCAN_EDGE_MAX_WIDTH         (5)       // index
#define SCAN_EDGES_MIN_DIST         (50)      // index
#define SCAN_EDGES_MAX_DIST         (70)      // index


class PuckScanner
{
public:
    PuckScanner(float y_min, float y_max) :
        m_left_sensor(I2C_ADDR_TOF_FOURCHE_AVG, PIN_EN_TOF_FOURCHE_AVG, SCAN_SENSOR_MIN, SCAN_SENSOR_MAX, "FourcheG", &Serial),
        m_right_sensor(I2C_ADDR_TOF_FOURCHE_AVD, PIN_EN_TOF_FOURCHE_AVD, SCAN_SENSOR_MIN, SCAN_SENSOR_MAX, "FourcheD", &Serial),
        m_y_min(y_min + SCAN_LEFT_SENSOR_POSITION), m_y_max(y_max + SCAN_RIGHT_SENSOR_POSITION)
    {
        m_scan_enabled = false;
    }

    int init()
    {
        int ret = EXIT_SUCCESS;
        if (m_left_sensor.powerON() != EXIT_SUCCESS) { ret = EXIT_FAILURE; }
        if (m_right_sensor.powerON() != EXIT_SUCCESS) { ret = EXIT_FAILURE; }
        return ret;
    }

    void reset() { m_raw_scan_data.clear(); }
    void enable(bool e) { m_scan_enabled = e; }

    void updateLeftSensor(SensorValue& s_val, float y)
    {
        registerSensorUpdate(m_left_sensor.getMeasure(), s_val, y + SCAN_LEFT_SENSOR_POSITION);
    }
    void updateRightSensor(SensorValue& s_val, float y)
    {
        registerSensorUpdate(m_right_sensor.getMeasure(), s_val, y + SCAN_RIGHT_SENSOR_POSITION);
    }

    int compute(float& y)
    {
        /* Index sensing values by their Y-coordinate */
        std::vector<int32_t> preprocess_data[SCAN_RESOLUTION];
        for (size_t i = 0; i < m_raw_scan_data.size(); i++) {
            int32_t d = m_raw_scan_data.at(i).distance;
            size_t idx = m_raw_scan_data.at(i).index;
            if (d >= 0 && idx < SCAN_RESOLUTION) {
                preprocess_data[idx].push_back(d);
            }
        }

        /* Merge sensing values with the same Y-coordinate and mark empty areas */
        int32_t scan_data[SCAN_RESOLUTION];
        for (size_t i = 0; i < SCAN_RESOLUTION; i++) {
            if (preprocess_data[i].size() == 0) {
                scan_data[i] = -1;
            }
            else {
                int32_t sum = 0;
                for (size_t j = 0; j < preprocess_data[i].size(); j++) {
                    sum += preprocess_data[i].at(j);
                }
                scan_data[i] = sum / preprocess_data[i].size();
            }
        }

        /* Fill empty areas with linear interpolation */
        size_t pre_index = findNextFilledPoint(scan_data, 0);
        size_t next_index = findNextFilledPoint(scan_data, pre_index + 1);
        if (pre_index < SCAN_RESOLUTION) {
            for (size_t i = 0; i < pre_index; i++) {
                scan_data[i] = scan_data[pre_index];
            }
        }
        else {
            return EXIT_FAILURE;
        }
        while (next_index < SCAN_RESOLUTION) {
            for (size_t i = pre_index + 1; i < next_index; i++) {
                scan_data[i] = interpolate(scan_data, i, pre_index, next_index);
            }
            pre_index = next_index;
            next_index = findNextFilledPoint(scan_data, next_index + 1);
        }
        for (size_t i = pre_index + 1; i < SCAN_RESOLUTION; i++) {
            scan_data[i] = scan_data[pre_index];
        }

        //Server.printf("index;distance\n");
        //for(size_t i = 0; i < SCAN_RESOLUTION; i++) {
        //    Server.printf("%u;%d\n", i, scan_data[i]);
        //}

        /* Edge detection */
        int32_t width = SCAN_EDGE_MAX_WIDTH;
        int32_t height = SCAN_EDGE_MIN_HEIGHT;
        int32_t puck_min_size = SCAN_EDGES_MIN_DIST;
        int32_t puck_max_size = SCAN_EDGES_MAX_DIST;
        int32_t puck_start_pos = 0;
        bool e = false;
        bool foundFallingEdge = false;
        for (size_t i = 0; i < SCAN_RESOLUTION - (size_t)width; i++) {
            int32_t delta = scan_data[i + width] - scan_data[i];
            if (!e && abs(delta) > height) {
                e = true;
                // New edge found
                if (foundFallingEdge && delta > 0) {
                    // Rising edge found after registering a falling edge
                    foundFallingEdge = false;
                    int32_t puck_end_pos = i + (width / 2);
                    int32_t puck_size = puck_end_pos - puck_start_pos;
                    if (puck_size < puck_max_size && puck_size > puck_min_size) {
                        // Puck found
                        y = indexToMm((puck_start_pos + puck_end_pos) / 2);
                        return EXIT_SUCCESS;
                    }
                }
                else if (delta < 0) {
                    // Registering the falling edge
                    foundFallingEdge = true;
                    puck_start_pos = i + (width / 2);
                }
            }
            else if (e && abs(delta) <= height) {
                e = false;
            }
        }
        // Puck not found
        return EXIT_FAILURE;
    }

private:
    void registerSensorUpdate(SensorValue input, SensorValue& output, float y)
    {
        if (input != SENSOR_NOT_UPDATED) {
            output = input;
            if (m_scan_enabled) {
                m_raw_scan_data.push_back(RawScanPoint(input, y, m_y_min, m_y_max));
            }
        }
    }

    size_t findNextFilledPoint(int32_t tab[SCAN_RESOLUTION], size_t start)
    {
        for (size_t i = start; i < SCAN_RESOLUTION; i++) {
            if (tab[i] >= 0) {
                return i;
            }
        }
        return SCAN_RESOLUTION;
    }

    int32_t interpolate(int32_t tab[SCAN_RESOLUTION], size_t index, size_t pre_index, size_t next_index)
    {
        float a = (float)(tab[next_index] - tab[pre_index]) / (float)(next_index - pre_index);
        return round((float)tab[pre_index] + (float)(index - pre_index) * a);
    }

    static int32_t mmToIndex(float y, float y_min, float y_max)
    {
        return round((constrain(y, y_min, y_max) - y_min) * (SCAN_RESOLUTION - 1) / (y_max - y_min));
    }

    int32_t mmToIndex(float y)
    {
        return mmToIndex(y, m_y_min, m_y_max);
    }

    float indexToMm(int32_t index)
    {
        return m_y_min + (float)index * (m_y_max - m_y_min) / ((float)SCAN_RESOLUTION - 1);
    }

    ToF_shortRange m_left_sensor;
    ToF_shortRange m_right_sensor;
    bool m_scan_enabled;
    const float m_y_min;
    const float m_y_max;

    struct RawScanPoint {
        RawScanPoint(SensorValue v, float y, float y_min, float y_max)
        {
            if (v == OBSTACLE_TOO_CLOSE) {
                distance = SCAN_SENSOR_MIN;
            }
            else if (v == NO_OBSTACLE) {
                distance = SCAN_SENSOR_MAX;
            }
            else if (v > SCAN_SENSOR_MAX || v < SCAN_SENSOR_MIN) {
                distance = -1;
            }
            else {
                distance = v;
            }
            index = mmToIndex(y, y_min, y_max);
        }
        size_t index;
        int32_t distance;
    };
    std::vector<RawScanPoint> m_raw_scan_data;

    struct Edge {
        Edge(bool rising, size_t position) {
            this->rising = rising;
            this->position = position;
        }
        bool rising;
        size_t position;
    };
};


#endif
