/**
 * BasicMinimal.ino
 * ────────────────
 * The simplest possible FIS example — great starting point.
 *
 * One input, one output, three rules.
 *
 * Maps a single sensor reading [0-100] to an LED brightness [0-255].
 *   low  reading → dim  LED
 *   mid  reading → med  LED
 *   high reading → bright LED
 *
 * Wiring:  Optional — connect an LED to LED_PIN (PWM).
 *          Results always printed to Serial at 115200 baud.
 */

#include <FIS.h>

#define LED_PIN 2   /* Built-in LED on many ESP32 boards */

/* Variables */
FIS_LinguisticVariable sensorVar;
FIS_LinguisticVariable ledVar;

/* Modules */
FIS_FuzzificationModule   fuzzifier;
FIS_InferenceModule       engine;
FIS_DefuzzificationModule defuzzifier;

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }
    pinMode(LED_PIN, OUTPUT);

    Serial.println("=============================================");
    Serial.println("  FIS Library - Basic Minimal Example");
    Serial.println("=============================================");

    /* ── Define input: sensor reading [0, 100] ────────────────────── */
    sensorVar.init("sensor", 0.0f, 100.0f);
    sensorVar.addTermTriangular("low",    0,  0, 50);
    sensorVar.addTermTriangular("mid",   25, 50, 75);
    sensorVar.addTermTriangular("high",  50, 100, 100);

    /* ── Define output: LED brightness [0, 255] ──────────────────── */
    ledVar.init("led", 0.0f, 255.0f);
    ledVar.addTermTriangular("dim",       0,   0,  128);
    ledVar.addTermTriangular("med",      64, 128,  192);
    ledVar.addTermTriangular("bright",  128, 255,  255);

    /* ── Register variables ───────────────────────────────────────── */
    int idxSensor = fuzzifier.addVariable(&sensorVar);
    defuzzifier.addOutputVariable(&ledVar);

    /* ── Define rules ─────────────────────────────────────────────── */
    /* Rule 1: IF sensor IS low THEN led IS dim */
    FIS_Rule r1;
    r1.addAntecedent(idxSensor, sensorVar.findTerm("low"));
    r1.setConsequent(0, ledVar.findTerm("dim"));
    engine.addRule(r1);

    /* Rule 2: IF sensor IS mid THEN led IS med */
    FIS_Rule r2;
    r2.addAntecedent(idxSensor, sensorVar.findTerm("mid"));
    r2.setConsequent(0, ledVar.findTerm("med"));
    engine.addRule(r2);

    /* Rule 3: IF sensor IS high THEN led IS bright */
    FIS_Rule r3;
    r3.addAntecedent(idxSensor, sensorVar.findTerm("high"));
    r3.setConsequent(0, ledVar.findTerm("bright"));
    engine.addRule(r3);

    /* ── Run the pipeline for several test values ─────────────────── */
    Serial.println();
    Serial.println("Sensor  -->  LED Brightness");
    Serial.println("------      ---------------");

    for (float val = 0; val <= 100; val += 10) {
        float inputs[1] = { val };
        float degrees[FIS_MAX_VARIABLES][FIS_MAX_TERMS];
        float outStrengths[FIS_MAX_VARIABLES][FIS_MAX_TERMS];
        float crispOut[1];

        fuzzifier.fuzzify(inputs, degrees);
        engine.infer(degrees, outStrengths, 1);
        defuzzifier.defuzzify(outStrengths, crispOut);

        int pwm = (int)constrain(crispOut[0], 0, 255);

        Serial.print("  ");
        if (val < 10) Serial.print(" ");
        if (val < 100) Serial.print(" ");
        Serial.print(val, 0);
        Serial.print("       -->   ");
        Serial.print(crispOut[0], 1);
        Serial.print("  (PWM ");
        Serial.print(pwm);
        Serial.println(")");

        analogWrite(LED_PIN, pwm);
        delay(200);
    }

    Serial.println("\nDone.");
}

void loop() { }
