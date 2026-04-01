/**
 * SmartIrrigation.ino
 * ───────────────────
 * Fuzzy irrigation controller for smart agriculture.
 *
 * Determines watering duration based on three sensor inputs:
 *   - Soil moisture level
 *   - Ambient temperature
 *   - Sunlight intensity
 *
 * This example shows how to build a 3-input, 1-output FIS,
 * and demonstrates the use of Gaussian membership functions.
 *
 * Inputs:
 *   - soil moisture [0-100%]   (0 = bone dry, 100 = saturated)
 *   - temperature   [0-50 °C]
 *   - sunlight      [0-1000 lux]
 *
 * Output:
 *   - watering duration [0-60 minutes]
 *
 * Wiring:  Simulated — results printed to Serial at 115200 baud.
 */

#include <FIS.h>

/* ── Linguistic variables ─────────────────────────────────────────── */
FIS_LinguisticVariable moistureVar;
FIS_LinguisticVariable tempVar;
FIS_LinguisticVariable sunVar;
FIS_LinguisticVariable waterVar;

/* ── FIS modules ──────────────────────────────────────────────────── */
FIS_FuzzificationModule   fuzzifier;
FIS_InferenceModule       engine;
FIS_DefuzzificationModule defuzzifier;

int idxMoist, idxTemp, idxSun;

void buildFIS() {

    /* ── Input 1: Soil Moisture [0, 100] % ────────────────────────── */
    moistureVar.init("moisture", 0.0f, 100.0f);
    moistureVar.addTermGaussian("dry",     15.0f, 12.0f);
    moistureVar.addTermGaussian("moist",   50.0f, 15.0f);
    moistureVar.addTermGaussian("wet",     85.0f, 12.0f);

    /* ── Input 2: Temperature [0, 50] °C ──────────────────────────── */
    tempVar.init("temp", 0.0f, 50.0f);
    tempVar.addTermTrapezoidal("cool",    -1,  0, 10, 20);
    tempVar.addTermTriangular( "mild",    15, 25, 35);
    tempVar.addTermTrapezoidal("hot",     30, 40, 50, 51);

    /* ── Input 3: Sunlight [0, 1000] lux ──────────────────────────── */
    sunVar.init("sun", 0.0f, 1000.0f);
    sunVar.addTermTrapezoidal("cloudy",   -1,   0, 150, 350);
    sunVar.addTermTriangular( "partial", 250, 500, 750);
    sunVar.addTermTrapezoidal("sunny",   650, 850, 1000, 1001);

    /* ── Output: Watering Duration [0, 60] min ────────────────────── */
    waterVar.init("water", 0.0f, 60.0f);
    waterVar.addTermTriangular("none",     0,  0,  10);
    waterVar.addTermTriangular("short",    5, 15,  25);
    waterVar.addTermTriangular("moderate",20, 30,  40);
    waterVar.addTermTriangular("long",    35, 50,  60);

    /* ── Register variables ───────────────────────────────────────── */
    idxMoist = fuzzifier.addVariable(&moistureVar);
    idxTemp  = fuzzifier.addVariable(&tempVar);
    idxSun   = fuzzifier.addVariable(&sunVar);
    defuzzifier.addOutputVariable(&waterVar);

    /* ── Term indices ─────────────────────────────────────────────── */
    int mDry   = moistureVar.findTerm("dry");
    int mMoist = moistureVar.findTerm("moist");
    int mWet   = moistureVar.findTerm("wet");

    int tCool = tempVar.findTerm("cool");
    int tMild = tempVar.findTerm("mild");
    int tHot  = tempVar.findTerm("hot");

    int sCloudy  = sunVar.findTerm("cloudy");
    int sPartial = sunVar.findTerm("partial");
    int sSunny   = sunVar.findTerm("sunny");

    int wNone = waterVar.findTerm("none");
    int wShort = waterVar.findTerm("short");
    int wMod   = waterVar.findTerm("moderate");
    int wLong  = waterVar.findTerm("long");

    /* ── Rules ────────────────────────────────────────────────────── */
    /* Dry soil always needs water — more if hot and sunny */

    /* Dry + Cool */
    FIS_Rule r1; r1.addAntecedent(idxMoist, mDry); r1.addAntecedent(idxTemp, tCool);
    r1.setConsequent(0, wShort); engine.addRule(r1);

    /* Dry + Mild */
    FIS_Rule r2; r2.addAntecedent(idxMoist, mDry); r2.addAntecedent(idxTemp, tMild);
    r2.setConsequent(0, wMod); engine.addRule(r2);

    /* Dry + Hot */
    FIS_Rule r3; r3.addAntecedent(idxMoist, mDry); r3.addAntecedent(idxTemp, tHot);
    r3.setConsequent(0, wLong); engine.addRule(r3);

    /* Dry + Sunny (regardless of temp) → extra long */
    FIS_Rule r4; r4.addAntecedent(idxMoist, mDry); r4.addAntecedent(idxSun, sSunny);
    r4.setConsequent(0, wLong); engine.addRule(r4);

    /* Moist + Cool → no watering */
    FIS_Rule r5; r5.addAntecedent(idxMoist, mMoist); r5.addAntecedent(idxTemp, tCool);
    r5.setConsequent(0, wNone); engine.addRule(r5);

    /* Moist + Mild → maybe short */
    FIS_Rule r6; r6.addAntecedent(idxMoist, mMoist); r6.addAntecedent(idxTemp, tMild);
    r6.setConsequent(0, wShort); engine.addRule(r6);

    /* Moist + Hot → moderate */
    FIS_Rule r7; r7.addAntecedent(idxMoist, mMoist); r7.addAntecedent(idxTemp, tHot);
    r7.setConsequent(0, wMod); engine.addRule(r7);

    /* Moist + Sunny → moderate */
    FIS_Rule r8; r8.addAntecedent(idxMoist, mMoist); r8.addAntecedent(idxSun, sSunny);
    r8.setConsequent(0, wMod); engine.addRule(r8);

    /* Wet → never water */
    FIS_Rule r9; r9.addAntecedent(idxMoist, mWet);
    r9.setConsequent(0, wNone); engine.addRule(r9);

    /* Cloudy in general → reduce watering (less evaporation) */
    FIS_Rule r10; r10.addAntecedent(idxSun, sCloudy); r10.addAntecedent(idxMoist, mMoist);
    r10.setConsequent(0, wNone); engine.addRule(r10);
}

float computeWatering(float moisture, float temp, float sun) {
    float inputs[3] = { moisture, temp, sun };
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

    Serial.println("=============================================");
    Serial.println("  FIS Library - Smart Irrigation Controller");
    Serial.println("=============================================");

    buildFIS();
    Serial.print("Rules: ");
    Serial.println(engine.ruleCount());
    Serial.println();

    /* Simulated scenarios */
    struct { float m; float t; float s; const char* desc; } tests[] = {
        { 10.0f, 38.0f, 900.0f, "Dry soil, hot, sunny"     },
        { 10.0f, 15.0f, 200.0f, "Dry soil, cool, cloudy"   },
        { 50.0f, 25.0f, 500.0f, "Moist soil, mild, partial"},
        { 50.0f, 42.0f, 850.0f, "Moist soil, hot, sunny"   },
        { 85.0f, 30.0f, 700.0f, "Wet soil, warm, sunny"    },
        { 30.0f, 20.0f, 100.0f, "Dry-ish, cool, cloudy"    },
        { 60.0f, 35.0f, 400.0f, "Moist, warm, partial sun" },
    };

    for (int i = 0; i < 7; i++) {
        float water = computeWatering(tests[i].m, tests[i].t, tests[i].s);
        Serial.print("[");
        Serial.print(tests[i].desc);
        Serial.print("]");
        Serial.print("  Moisture=");
        Serial.print(tests[i].m, 0);
        Serial.print("%, Temp=");
        Serial.print(tests[i].t, 0);
        Serial.print("C, Sun=");
        Serial.print(tests[i].s, 0);
        Serial.print("lux  -->  Water = ");
        Serial.print(water, 1);
        Serial.println(" min");
    }

    Serial.println("\nDone.");
}

void loop() { }
