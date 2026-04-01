/**
 * TemperatureFanControl.ino
 * ─────────────────────────
 * Fuzzy fan speed controller based on temperature and humidity.
 *
 * A practical HVAC-style example: reads temperature (°C) and humidity (%),
 * then uses fuzzy logic to determine an appropriate fan speed (PWM 0-255).
 *
 * In this demo, sensor values are simulated. Replace with real sensor
 * readings (e.g. DHT22, BME280) for a real application.
 *
 * Inputs:  temperature [0-50 °C], humidity [0-100 %]
 * Output:  fan speed [0-255] (PWM duty cycle)
 *
 * Wiring:  Connect a fan/LED to FAN_PIN (PWM) to see the result,
 *          or just read values from Serial Monitor at 115200 baud.
 */

#include <FIS.h>

#define FAN_PIN 25   /* GPIO for PWM output (ESP32 example) */

/* ── Linguistic variables ─────────────────────────────────────────── */
FIS_LinguisticVariable tempVar;
FIS_LinguisticVariable humVar;
FIS_LinguisticVariable fanVar;

/* ── FIS modules ──────────────────────────────────────────────────── */
FIS_FuzzificationModule   fuzzifier;
FIS_InferenceModule       engine;
FIS_DefuzzificationModule defuzzifier;

/* ── Indices ──────────────────────────────────────────────────────── */
int idxTemp, idxHum;
int idxFanOut;

void buildFIS() {

    /* ── Input 1: Temperature [0, 50] °C ──────────────────────────── */
    tempVar.init("temp", 0.0f, 50.0f);
    tempVar.addTermTrapezoidal("cold",    -1,  0,  10, 18);
    tempVar.addTermTriangular( "warm",    15,  25, 35);
    tempVar.addTermTrapezoidal("hot",     30,  40, 50, 51);

    /* ── Input 2: Humidity [0, 100] % ─────────────────────────────── */
    humVar.init("hum", 0.0f, 100.0f);
    humVar.addTermTrapezoidal("dry",     -1,   0,  20, 40);
    humVar.addTermTriangular( "normal",  30,   50, 70);
    humVar.addTermTrapezoidal("humid",   60,   80, 100, 101);

    /* ── Output: Fan Speed [0, 255] ───────────────────────────────── */
    fanVar.init("fan", 0.0f, 255.0f);
    fanVar.addTermTriangular("off",      0,    0,   64);
    fanVar.addTermTriangular("low",     32,   80,  128);
    fanVar.addTermTriangular("medium", 100,  150,  200);
    fanVar.addTermTriangular("high",   170,  230,  255);

    /* ── Register variables ───────────────────────────────────────── */
    idxTemp = fuzzifier.addVariable(&tempVar);
    idxHum  = fuzzifier.addVariable(&humVar);
    idxFanOut = defuzzifier.addOutputVariable(&fanVar);

    /* ── Term indices ─────────────────────────────────────────────── */
    int tCold = tempVar.findTerm("cold");
    int tWarm = tempVar.findTerm("warm");
    int tHot  = tempVar.findTerm("hot");

    int tDry    = humVar.findTerm("dry");
    int tNormal = humVar.findTerm("normal");
    int tHumid  = humVar.findTerm("humid");

    int fOff    = fanVar.findTerm("off");
    int fLow    = fanVar.findTerm("low");
    int fMed    = fanVar.findTerm("medium");
    int fHigh   = fanVar.findTerm("high");

    /* ── Rules ────────────────────────────────────────────────────── */

    /* R1: IF temp IS cold AND hum IS dry THEN fan IS off */
    FIS_Rule r1;
    r1.addAntecedent(idxTemp, tCold);
    r1.addAntecedent(idxHum,  tDry);
    r1.setConsequent(0, fOff);
    engine.addRule(r1);

    /* R2: IF temp IS cold AND hum IS normal THEN fan IS off */
    FIS_Rule r2;
    r2.addAntecedent(idxTemp, tCold);
    r2.addAntecedent(idxHum,  tNormal);
    r2.setConsequent(0, fOff);
    engine.addRule(r2);

    /* R3: IF temp IS cold AND hum IS humid THEN fan IS low */
    FIS_Rule r3;
    r3.addAntecedent(idxTemp, tCold);
    r3.addAntecedent(idxHum,  tHumid);
    r3.setConsequent(0, fLow);
    engine.addRule(r3);

    /* R4: IF temp IS warm AND hum IS dry THEN fan IS low */
    FIS_Rule r4;
    r4.addAntecedent(idxTemp, tWarm);
    r4.addAntecedent(idxHum,  tDry);
    r4.setConsequent(0, fLow);
    engine.addRule(r4);

    /* R5: IF temp IS warm AND hum IS normal THEN fan IS medium */
    FIS_Rule r5;
    r5.addAntecedent(idxTemp, tWarm);
    r5.addAntecedent(idxHum,  tNormal);
    r5.setConsequent(0, fMed);
    engine.addRule(r5);

    /* R6: IF temp IS warm AND hum IS humid THEN fan IS high */
    FIS_Rule r6;
    r6.addAntecedent(idxTemp, tWarm);
    r6.addAntecedent(idxHum,  tHumid);
    r6.setConsequent(0, fHigh);
    engine.addRule(r6);

    /* R7: IF temp IS hot AND hum IS dry THEN fan IS medium */
    FIS_Rule r7;
    r7.addAntecedent(idxTemp, tHot);
    r7.addAntecedent(idxHum,  tDry);
    r7.setConsequent(0, fMed);
    engine.addRule(r7);

    /* R8: IF temp IS hot AND hum IS normal THEN fan IS high */
    FIS_Rule r8;
    r8.addAntecedent(idxTemp, tHot);
    r8.addAntecedent(idxHum,  tNormal);
    r8.setConsequent(0, fHigh);
    engine.addRule(r8);

    /* R9: IF temp IS hot AND hum IS humid THEN fan IS high */
    FIS_Rule r9;
    r9.addAntecedent(idxTemp, tHot);
    r9.addAntecedent(idxHum,  tHumid);
    r9.setConsequent(0, fHigh);
    engine.addRule(r9);
}

float computeFanSpeed(float temperature, float humidity) {
    float inputs[2] = { temperature, humidity };
    float degrees[FIS_MAX_VARIABLES][FIS_MAX_TERMS];
    float outStrengths[FIS_MAX_VARIABLES][FIS_MAX_TERMS];
    float crispOut[1];

    fuzzifier.fuzzify(inputs, degrees);
    engine.infer(degrees, outStrengths, 1);
    defuzzifier.defuzzify(outStrengths, crispOut);

    return crispOut[0];
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }

    pinMode(FAN_PIN, OUTPUT);

    Serial.println("=============================================");
    Serial.println("  FIS Library - Temperature Fan Control");
    Serial.println("=============================================");

    buildFIS();

    /* Test with various simulated sensor readings */
    float testCases[][2] = {
        {  5.0f, 20.0f },   /* Cold & dry   → fan off      */
        { 25.0f, 50.0f },   /* Warm & normal → fan medium   */
        { 42.0f, 85.0f },   /* Hot & humid   → fan high     */
        { 30.0f, 30.0f },   /* Warm-hot & dry-normal        */
        { 15.0f, 70.0f },   /* Cold-warm & humid            */
    };

    for (int i = 0; i < 5; i++) {
        float temp = testCases[i][0];
        float hum  = testCases[i][1];
        float fan  = computeFanSpeed(temp, hum);

        Serial.println();
        Serial.print("Temp=");
        Serial.print(temp, 1);
        Serial.print(" C, Humidity=");
        Serial.print(hum, 1);
        Serial.print(" %  -->  Fan PWM = ");
        Serial.println(fan, 1);

        /* Write to actual PWM pin */
        analogWrite(FAN_PIN, (int)constrain(fan, 0, 255));
        delay(500);
    }

    Serial.println();
    Serial.println("Done. In a real application, read sensors in loop().");
}

void loop() {
    /* In a real application you would read sensors here:
     *
     * float temp = readTemperatureSensor();
     * float hum  = readHumiditySensor();
     * float fan  = computeFanSpeed(temp, hum);
     * analogWrite(FAN_PIN, (int)constrain(fan, 0, 255));
     * delay(1000);
     */
}
