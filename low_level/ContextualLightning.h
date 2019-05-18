#ifndef CONTEXTUAL_LIGHTNING_h
#define CONTEXTUAL_LIGHTNING_h

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Printable.h>
#include <vector>
#include "MotionControlSystem.h"
#include "LedBlinker.h"
#include "Config.h"

#define CONTEXTUAL_LIGHTNING_UPDATE_PERIOD  50      // ms
#define NB_NEOPIXELS_FRONT                  16
#define NB_NEOPIXELS_BACK                   14
#define TURNING_THRESHOLD                   0.5     // m^-1
#define SIDE_DISPLAY_DURATION               4000    // ms

/* Color definition */
#define COLOR_ORANGE                (Adafruit_NeoPixel::Color(102, 25, 0, 0))
#define COLOR_VIOLET                (Adafruit_NeoPixel::Color(23, 0, 105, 0))
#define COLOR_RED_DIM               (Adafruit_NeoPixel::Color(10, 0, 0, 0))
#define COLOR_RED_BRIGHT            (Adafruit_NeoPixel::Color(200, 0, 0, 0))
#define COLOR_WHITE_DIM             (Adafruit_NeoPixel::Color(0, 0, 0, 10))
#define COLOR_WHITE_BRIGHT          (Adafruit_NeoPixel::Color(0, 0, 0, 70))
#define COLOR_WHITE_ULTRA_BRIGHT    (Adafruit_NeoPixel::Color(0, 0, 0, 255))


class NeoPixelSubGroup
{
public:
    NeoPixelSubGroup(uint32_t c, uint16_t led_index_range, uint32_t off_d = 0, uint32_t on_d = 0)
    {
        enabled = false;
        color = c;
        blinker.setPeriod(off_d, on_d);
        for (uint16_t i = 0; i < led_index_range; i++) {
            led_index.push_back(i);
        }
    }

    NeoPixelSubGroup(uint32_t c, const std::vector<uint16_t> &led_index, uint32_t off_d = 0, uint32_t on_d = 0) :
        led_index(led_index)
    {
        enabled = false;
        color = c;
        blinker.setPeriod(off_d, on_d);
    }

    void enable(bool e)
    {
        if (e && !enabled) {
            blinker.start();
        }
        enabled = e;
    }

    void writePixels(Adafruit_NeoPixel &pixels) const
    {
        if (enabled) {
            uint32_t c = 0;
            if (blinker.value()) {
                c = color;
            }
            for (size_t i = 0; i < led_index.size(); i++) {
                pixels.setPixelColor(led_index.at(i), c);
            }
        }
    }

private:
    bool enabled;
    LedBlinker blinker;
    uint32_t color;
    std::vector<uint16_t> led_index;
};


class NeoPixelGroup
{
public:
    NeoPixelGroup(uint16_t pixel_count, uint8_t pin, std::vector<NeoPixelSubGroup> zones) :
        pixels(pixel_count, pin, NEO_GRBW + NEO_KHZ800),
        zones(zones)
    {
        pixels.begin();
    }

    void enableZone(size_t zone, bool enable)
    {
        if (zone < zones.size()) {
            zones.at(zone).enable(enable);
        }
    }

    void update()
    {
        pixels.clear();
        for (size_t i = 0; i < zones.size(); i++) {
            zones.at(i).writePixels(pixels);
        }
        pixels.show();
    }

private:
    Adafruit_NeoPixel pixels;
    std::vector<NeoPixelSubGroup> zones;
};


class ContextualLightning : public Singleton<ContextualLightning>, public Printable
{
private:
    enum Zone {
        ZONE_NIGHT_LIGHT_BACK = 0,
        ZONE_NIGHT_LIGHT_FRONT_DIM = 1,
        ZONE_NIGHT_LIGHT_FRONT_BRIGHT = 2,
        ZONE_NIGHT_LIGHT_FRONT_ULTRA_BRIGHT = 3,
        ZONE_BREAKING_LIGHT = 4,
        ZONE_BLINKER_RIGHT = 5,
        ZONE_BLINKER_LEFT = 6,
        ZONE_DISPLAY_ORANGE = 7,
        ZONE_DISPLAY_VIOLET = 8
    };

    enum Blinkers {
        BLINKERS_OFF,
        BLINKERS_LEFT,
        BLINKERS_RIGHT,
        BLINKERS_BOTH
    };

public:
    enum NightLight {
        NIGHT_LIGHT_OFF = 0,
        NIGHT_LIGHT_LOW = 1,
        NIGHT_LIGHT_MID = 2,
        NIGHT_LIGHT_MAX = 3,
    };

    enum SideDisplay {
        SIDE_DISPLAY_OFF    = 0,
        SIDE_DISPLAY_ORANGE = 1,
        SIDE_DISPLAY_VIOLET = 2
    };

    ContextualLightning() :
        motionControlSystem(MotionControlSystem::Instance()),
        lightGroupA(NB_NEOPIXELS_FRONT, PIN_NEOPIXELS_FRONT,
            std::vector<NeoPixelSubGroup>({
                /* ZONE_NIGHT_LIGHT_BACK */
                NeoPixelSubGroup(COLOR_RED_DIM, std::vector<uint16_t>({ 0, 1, 2, 3, 12, 13, 14, 15 })),
                /* ZONE_NIGHT_LIGHT_FRONT_DIM */
                NeoPixelSubGroup(COLOR_WHITE_DIM, NB_NEOPIXELS_FRONT),
                /* ZONE_NIGHT_LIGHT_FRONT_BRIGHT */
                NeoPixelSubGroup(COLOR_WHITE_BRIGHT, NB_NEOPIXELS_FRONT),
                /* ZONE_NIGHT_LIGHT_FRONT_ULTRA_BRIGHT */
                NeoPixelSubGroup(COLOR_WHITE_ULTRA_BRIGHT, NB_NEOPIXELS_FRONT),
                /* ZONE_BREAKING_LIGHT */
                NeoPixelSubGroup(COLOR_RED_BRIGHT, std::vector<uint16_t>({ 0, 1, 2, 3, 4, 11, 12, 13, 14, 15 })),
                /* ZONE_BLINKER_RIGHT */
                NeoPixelSubGroup(COLOR_ORANGE, std::vector<uint16_t>({ 14, 15 }), 400, 400),
                /* ZONE_BLINKER_LEFT */
                NeoPixelSubGroup(COLOR_ORANGE, std::vector<uint16_t>({ 0, 1 }), 400, 400),
                /* ZONE_DISPLAY_ORANGE */
                NeoPixelSubGroup(COLOR_ORANGE, NB_NEOPIXELS_FRONT, 200, 800),
                /* ZONE_DISPLAY_VIOLET */
                NeoPixelSubGroup(COLOR_VIOLET, NB_NEOPIXELS_FRONT, 200, 800)
            })),
        lightGroupB(NB_NEOPIXELS_BACK, PIN_NEOPIXELS_BACK,
            std::vector<NeoPixelSubGroup>({
                /* ZONE_NIGHT_LIGHT_BACK */
                NeoPixelSubGroup(COLOR_RED_DIM, std::vector<uint16_t>({ 0, 1, 2, 3, 4, 7, 8, 9, 10, 11 })),
                /* ZONE_NIGHT_LIGHT_FRONT_DIM */
                NeoPixelSubGroup(COLOR_WHITE_DIM, NB_NEOPIXELS_BACK),
                /* ZONE_NIGHT_LIGHT_FRONT_BRIGHT */
                NeoPixelSubGroup(COLOR_WHITE_BRIGHT, NB_NEOPIXELS_BACK),
                /* ZONE_NIGHT_LIGHT_FRONT_ULTRA_BRIGHT */
                NeoPixelSubGroup(COLOR_WHITE_ULTRA_BRIGHT, NB_NEOPIXELS_BACK),
                /* ZONE_BREAKING_LIGHT */
                NeoPixelSubGroup(COLOR_RED_BRIGHT, NB_NEOPIXELS_BACK),
                /* ZONE_BLINKER_RIGHT */
                NeoPixelSubGroup(COLOR_ORANGE, std::vector<uint16_t>({ 8, 9, 13 }), 400, 400),
                /* ZONE_BLINKER_LEFT */
                NeoPixelSubGroup(COLOR_ORANGE, std::vector<uint16_t>({ 3, 4, 5 }), 400, 400),
                /* ZONE_DISPLAY_ORANGE */
                NeoPixelSubGroup(COLOR_ORANGE, NB_NEOPIXELS_BACK, 200, 800),
                /* ZONE_DISPLAY_VIOLET */
                NeoPixelSubGroup(COLOR_VIOLET, NB_NEOPIXELS_BACK, 200, 800)
            }))
    {
        blinkers = BLINKERS_OFF;
        nightLight = NIGHT_LIGHT_OFF;
        sideDisplay = SIDE_DISPLAY_OFF;
        movingForward = true;
        breaking = false;
        sideDisplayStartTime = 0;
    }

    void update()
    {
        static uint32_t lastUpdateTime = 0;
        uint32_t now = millis();
        if (now - lastUpdateTime > CONTEXTUAL_LIGHTNING_UPDATE_PERIOD)
        {
            lastUpdateTime = now;
            if (now - sideDisplayStartTime > SIDE_DISPLAY_DURATION) {
                sideDisplay = SIDE_DISPLAY_OFF;
            }

            int dir = motionControlSystem.getMovingDirection();
            if (dir > 0) {
                movingForward = true;
            }
            else if (dir < 0) {
                movingForward = false;
            }
            breaking = motionControlSystem.parkingBreakEnabled() || motionControlSystem.isBreaking();
            float curvature = motionControlSystem.getCurvature();
            if (blinkers != BLINKERS_BOTH) {
                if (curvature < -TURNING_THRESHOLD) {
                    blinkers = BLINKERS_RIGHT;
                }
                else if (curvature > TURNING_THRESHOLD) {
                    blinkers = BLINKERS_LEFT;
                }
                else {
                    blinkers = BLINKERS_OFF;
                }
            }

            NeoPixelGroup *front;
            NeoPixelGroup *rear;
            if (movingForward) {
                front = &lightGroupA;
                rear = &lightGroupB;
            }
            else {
                front = &lightGroupB;
                rear = &lightGroupA;
            }

            front->enableZone(ZONE_NIGHT_LIGHT_BACK, false);
            rear->enableZone(ZONE_NIGHT_LIGHT_BACK, nightLight != NIGHT_LIGHT_OFF);
            front->enableZone(ZONE_NIGHT_LIGHT_FRONT_DIM, nightLight == NIGHT_LIGHT_LOW);
            rear->enableZone(ZONE_NIGHT_LIGHT_FRONT_DIM, false);
            front->enableZone(ZONE_NIGHT_LIGHT_FRONT_BRIGHT, nightLight == NIGHT_LIGHT_MID);
            rear->enableZone(ZONE_NIGHT_LIGHT_FRONT_BRIGHT, false);
            front->enableZone(ZONE_NIGHT_LIGHT_FRONT_ULTRA_BRIGHT, nightLight == NIGHT_LIGHT_MAX);
            rear->enableZone(ZONE_NIGHT_LIGHT_FRONT_ULTRA_BRIGHT, false);
            front->enableZone(ZONE_BREAKING_LIGHT, false);
            rear->enableZone(ZONE_BREAKING_LIGHT, breaking);
            front->enableZone(ZONE_BLINKER_LEFT, blinkers == BLINKERS_LEFT || blinkers == BLINKERS_BOTH);
            rear->enableZone(ZONE_BLINKER_LEFT, blinkers == BLINKERS_LEFT || blinkers == BLINKERS_BOTH);
            front->enableZone(ZONE_BLINKER_RIGHT, blinkers == BLINKERS_RIGHT || blinkers == BLINKERS_BOTH);
            rear->enableZone(ZONE_BLINKER_RIGHT, blinkers == BLINKERS_RIGHT || blinkers == BLINKERS_BOTH);
            front->enableZone(ZONE_DISPLAY_ORANGE, sideDisplay == SIDE_DISPLAY_ORANGE);
            rear->enableZone(ZONE_DISPLAY_ORANGE, sideDisplay == SIDE_DISPLAY_ORANGE);
            front->enableZone(ZONE_DISPLAY_VIOLET, sideDisplay == SIDE_DISPLAY_VIOLET);
            rear->enableZone(ZONE_DISPLAY_VIOLET, sideDisplay == SIDE_DISPLAY_VIOLET);
            
            lightGroupA.update();
            lightGroupB.update();
        }
    }

    void setNightLight(NightLight mode)
    {
        nightLight = mode;
    }

    void setSideDisplay(SideDisplay mode)
    {
        sideDisplay = mode;
        sideDisplayStartTime = millis();
    }

    void enableWarnings(bool enable)
    {
        if (enable) {
            blinkers = BLINKERS_BOTH;
            lightGroupA.enableZone(ZONE_BLINKER_LEFT, false);
            lightGroupB.enableZone(ZONE_BLINKER_LEFT, false);
            lightGroupA.enableZone(ZONE_BLINKER_RIGHT, false);
            lightGroupB.enableZone(ZONE_BLINKER_RIGHT, false);
        }
        else {
            blinkers = BLINKERS_OFF;
        }
    }

    size_t printTo(Print& p) const
    {
        return 0;
    }

private:
    const MotionControlSystem & motionControlSystem;
    NeoPixelGroup lightGroupA; // Horizontal bar (front)
    NeoPixelGroup lightGroupB; // Two circles (rear)

    Blinkers blinkers;
    NightLight nightLight;
    SideDisplay sideDisplay;
    bool movingForward;
    bool breaking;
    uint32_t sideDisplayStartTime;
};


#endif
