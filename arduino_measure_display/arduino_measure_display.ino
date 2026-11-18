#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <LiquidCrystal_I2C.h>

// Task declarations
void TaskMeasure(void *pvParameters);
void TaskDisplay(void *pvParameters);
void TaskButton(void *pvParameters);

QueueHandle_t distanceQueue;
SemaphoreHandle_t measureSemaphore;

constexpr uint8_t TRIG_PIN = 9;
constexpr uint8_t ECHO_PIN = 10;
constexpr uint8_t BUTTON_PIN = 2;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup()
{
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(BUTTON_PIN, INPUT);

    digitalWrite(TRIG_PIN, LOW);

    lcd.init();
    lcd.backlight();
    lcd.clear();

    // A length-one queue allows xQueueOverwrite().
    distanceQueue = xQueueCreate(1, sizeof(float));

    if (distanceQueue == NULL)
    {
        lcd.print("Queue error");
        
        for (;;)
        {
        }
    }

    measureSemaphore = xSemaphoreCreateBinary();

    if (measureSemaphore == NULL) {
        lcd.print("Semaphore error");

        for (;;) {
            
        }
    }

    BaseType_t measureCreated = xTaskCreate(
        TaskMeasure,   // Task function
        "Measure",     // Debugging name
        192,           // Stack allocation
        NULL,          // No task parameter
        1,             // Priority
        NULL           // Do not save a task handle
    );

    if (measureCreated != pdPASS)
    {
        lcd.clear();
        lcd.print("Measure error");

        for (;;)
        {
        }
    }

    BaseType_t displayCreated = xTaskCreate(
        TaskDisplay,
        "Display",
        192,
        NULL,
        1,
        NULL
    );

    if (displayCreated != pdPASS)
    {
        lcd.clear();
        lcd.print("Display error");

        for (;;)
        {
        }
    }

    BaseType_t buttonCreated = xTaskCreate(
        TaskButton,
        "Button",
        128,
        NULL,
        2,  // respond to button presses promptly
        NULL
    );

    if (buttonCreated != pdPASS) {
        lcd.clear();
        lcd.print("Button error");

        for (;;) {

        }
    }
}

void TaskMeasure(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        // This task consumes no CPU while waiting for a button press.
        if (xSemaphoreTake(measureSemaphore, portMAX_DELAY) == pdTRUE)
        {
            digitalWrite(TRIG_PIN, LOW);
            delayMicroseconds(2);

            digitalWrite(TRIG_PIN, HIGH);
            delayMicroseconds(10);

            digitalWrite(TRIG_PIN, LOW);

            unsigned long duration =
                pulseIn(ECHO_PIN, HIGH, 30000UL);

            if (duration > 0)
            {
                float distance = (duration / 2.0f) / 74.0f;

                xQueueOverwrite(distanceQueue, &distance);
            }
        }
    }
}

void TaskDisplay(void *pvParameters)
{
    (void)pvParameters;

    float receivedDistance;

    for (;;)
    {
        // Wait indefinitely for the next button measurement.
        if (xQueueReceive(
                distanceQueue,
                &receivedDistance,
                portMAX_DELAY) == pdTRUE)
        {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(receivedDistance, 1);
            lcd.print(" in");
        }
    }
}

void TaskButton(void *pvParameters) {
    (void)pvParameters;

    bool readyForPress = true;

    for (;;) {
        int buttonState = digitalRead(BUTTON_PIN);

        if (readyForPress && buttonState == HIGH) {
            readyForPress = false;

            // Tell the measurement task to take one reading
            xSemaphoreGive(measureSemaphore);
        }

        if (!readyForPress && buttonState == LOW) {
            vTaskDelay(pdMS_TO_TICKS(50));

            if (digitalRead(BUTTON_PIN) == LOW) {
                readyForPress = true;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void loop()
{
    // Arduino_FreeRTOS runs the tasks created in setup().
}
