/*********************************************************************
 * Task 5: Unloading (Potato / Ball Sorting)
 *
 * - Uses barcode (binary) from previous task.
 * - If number is even:
 *      good  -> RED basket
 *      bad   -> BLUE basket
 *   If number is odd:
 *      good  -> BLUE basket
 *      bad   -> RED basket
 *
 * - BLUE basket   = BallCollector.sortingPos0  (left)
 * - HOME (center) = BallCollector.sortingPos1
 * - RED basket    = BallCollector.sortingPos2  (right)
 *
 * Change mapping in the .cpp file if your hardware is opposite.
 *********************************************************************/

#ifndef TASK5_UNLOADING_H
#define TASK5_UNLOADING_H

#include <Arduino.h>

// Servo pin definitions
#define UNLOADING_Servo_PIN 39

// ---------------- Task 5 Sub-States ----------------
enum Task5SubState {
  T5_INIT,
  T5_NAVIGATE_TO_ZONE,
  T5_ALIGN,
  T5_UNLOADING,
  T5_VERIFY,
  T5_COMPLETED
};

// ---------------- Potato / Ball Types --------------
enum PotatoQuality {
  POTATO_GOOD,
  POTATO_BAD
};

enum BasketColor {
  BASKET_RED,
  BASKET_BLUE
};

class Task5Unloading {
  private:
    Task5SubState currentSubState;
    unsigned long subStateStartTime;
    bool taskActive;
    int  ballsUnloaded;

    // Motion / timing configuration
    unsigned long unloadDuration;
    unsigned long alignDuration;
    uint16_t zoneDetectionDistance;
    uint16_t targetBallCount;

    // Barcode + sorting logic
    String        barcodeBinary;    // e.g. "1010"
    uint16_t      barcodeValue;     // converted decimal value
    bool          barcodeValid;
    PotatoQuality potatoQuality;
    BasketColor   targetBasket;
    bool          hasSortedThisCycle;

    // Internal helpers
    uint16_t binaryToValue(const String &bin) const;
    BasketColor chooseBasket() const;

  public:
    Task5Unloading();

    // Core task API
    void init();
    void execute();
    void updateDisplay();
    void setSubState(Task5SubState newSubState);
    Task5SubState getSubState();
    String getSubStateName();
    void start();
    void stop();
    bool isActive();
    bool isCompleted();
    void reset();
    int  getBallsUnloaded();

    // --- Sorting configuration from previous tasks ---

    // Give barcode as binary string (e.g. "1010")
    void setBarcodeBinary(const String &binary);

    // Or directly as integer (e.g. 10)
    void setBarcodeValue(uint16_t value);

    // Tell Task 5 whether the current item is GOOD or BAD
    void setPotatoQuality(PotatoQuality q);

    // For debugging
    uint16_t   getBarcodeValue() const { return barcodeValue; }
    String     getBarcodeBinary() const { return barcodeBinary; }
    PotatoQuality getPotatoQuality() const { return potatoQuality; }
    BasketColor   getTargetBasket() const { return targetBasket; }

    // Configuration setters (timing / detection)
    void setUnloadDuration(unsigned long timeMs);
    void setAlignDuration(unsigned long timeMs);
    void setZoneDetectionDistance(uint16_t distance);
    void setTargetBallCount(uint16_t count);

    // Configuration getters
    unsigned long getUnloadDuration();
    unsigned long getAlignDuration();
    uint16_t getZoneDetectionDistance();
    uint16_t getTargetBallCount();
};

// Global instance
extern Task5Unloading task5Unloading;

#endif

