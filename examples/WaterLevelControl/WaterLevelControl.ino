/**
 * WaterLevelControl.ino
 * ─────────────────────
 * Fuzzy controller for a water tank pump system.
 *
 * Uses fuzzy logic to determine pump speed based on the current
 * water level and the rate of level change (rising or falling).
 *
 * Inputs:  water level [0-100 cm], level change rate [-10 to +10 cm/s]
 * Output:  pump valve opening [0-100 %]
 *
 * A negative change rate means water is dropping (pump should work harder).
 * A positive change rate means water is rising (pump can back off).
 *
 * Wiring:  Simulated — results printed to Serial at 115200 baud.
 */

#include <FIS.h>

/* ── Linguistic variables ─────────────────────────────────────────── */
FIS_LinguisticVariable levelVar;
FIS_LinguisticVariable rateVar;
FIS_LinguisticVariable valveVar;

/* ── FIS modules ──────────────────────────────────────────────────── */
FIS_FuzzificationModule   fuzzifier;
FIS_InferenceModule       engine;
FIS_DefuzzificationModule defuzzifier;

int idxLevel, idxRate;

void buildFIS() {

    /* ── Input 1: Water Level [0, 100] cm ─────────────────────────── */
    levelVar.init("level", 0.0f, 100.0f);
    levelVar.addTermTrapezoidal("low",     -1,   0,  15,  35);
    levelVar.addTermTriangular( "medium",  25,  50,  75);
    levelVar.addTermTrapezoidal("high",    65,  85, 100, 101);

    /* ── Input 2: Rate of Change [-10, 10] cm/s ───────────────────── */
    /*    negative = dropping, positive = rising                        */
    rateVar.init("rate", -10.0f, 10.0f);
    rateVar.addTermTrapezoidal("falling", -11, -10,  -5,  -1);
    rateVar.addTermTriangular( "steady",   -3,   0,   3);
    rateVar.addTermTrapezoidal("rising",    1,   5,  10,  11);

    /* ── Output: Valve Opening [0, 100] % ─────────────────────────── */
    valveVar.init("valve", 0.0f, 100.0f);
    valveVar.addTermTriangular("closed",    0,   0,  25);
    valveVar.addTermTriangular("partial",  15,  40,  65);
    valveVar.addTermTriangular("half",     40,  55,  70);
    valveVar.addTermTriangular("wide",     60,  80, 100);
    valveVar.addTermTriangular("full",     80, 100, 100);

    /* ── Register variables ───────────────────────────────────────── */
    idxLevel = fuzzifier.addVariable(&levelVar);
    idxRate  = fuzzifier.addVariable(&rateVar);
    defuzzifier.addOutputVariable(&valveVar);

    /* ── Term indices ─────────────────────────────────────────────── */
    int lLow  = levelVar.findTerm("low");
    int lMed  = levelVar.findTerm("medium");
    int lHigh = levelVar.findTerm("high");

    int rFall   = rateVar.findTerm("falling");
    int rSteady = rateVar.findTerm("steady");
    int rRise   = rateVar.findTerm("rising");

    int vClosed  = valveVar.findTerm("closed");
    int vPartial = valveVar.findTerm("partial");
    int vHalf    = valveVar.findTerm("half");
    int vWide    = valveVar.findTerm("wide");
    int vFull    = valveVar.findTerm("full");

    /* ── Rules ────────────────────────────────────────────────────── */
    /* If level is low, pump hard regardless of rate */
    FIS_Rule r1; r1.addAntecedent(idxLevel, lLow); r1.addAntecedent(idxRate, rFall);
    r1.setConsequent(0, vFull); engine.addRule(r1);

    FIS_Rule r2; r2.addAntecedent(idxLevel, lLow); r2.addAntecedent(idxRate, rSteady);
    r2.setConsequent(0, vWide); engine.addRule(r2);

    FIS_Rule r3; r3.addAntecedent(idxLevel, lLow); r3.addAntecedent(idxRate, rRise);
    r3.setConsequent(0, vHalf); engine.addRule(r3);

    /* If level is medium, adjust based on rate */
    FIS_Rule r4; r4.addAntecedent(idxLevel, lMed); r4.addAntecedent(idxRate, rFall);
    r4.setConsequent(0, vWide); engine.addRule(r4);

    FIS_Rule r5; r5.addAntecedent(idxLevel, lMed); r5.addAntecedent(idxRate, rSteady);
    r5.setConsequent(0, vPartial); engine.addRule(r5);

    FIS_Rule r6; r6.addAntecedent(idxLevel, lMed); r6.addAntecedent(idxRate, rRise);
    r6.setConsequent(0, vClosed); engine.addRule(r6);

    /* If level is high, slow down or stop */
    FIS_Rule r7; r7.addAntecedent(idxLevel, lHigh); r7.addAntecedent(idxRate, rFall);
    r7.setConsequent(0, vPartial); engine.addRule(r7);

    FIS_Rule r8; r8.addAntecedent(idxLevel, lHigh); r8.addAntecedent(idxRate, rSteady);
    r8.setConsequent(0, vClosed); engine.addRule(r8);

    FIS_Rule r9; r9.addAntecedent(idxLevel, lHigh); r9.addAntecedent(idxRate, rRise);
    r9.setConsequent(0, vClosed); engine.addRule(r9);
}

float computeValve(float level, float rate) {
    float inputs[2] = { level, rate };
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
    Serial.println("  FIS Library - Water Level Pump Control");
    Serial.println("=============================================");

    buildFIS();

    /* Simulated scenarios */
    struct { float level; float rate; const char* desc; } tests[] = {
        { 10.0f, -5.0f, "Low level, dropping fast"  },
        { 10.0f,  3.0f, "Low level, rising"          },
        { 50.0f,  0.0f, "Mid level, steady"          },
        { 50.0f, -7.0f, "Mid level, dropping fast"   },
        { 85.0f,  2.0f, "High level, still rising"   },
        { 90.0f, -1.0f, "High level, dropping slowly" },
        { 30.0f,  0.0f, "Low-mid, steady"            },
    };

    for (int i = 0; i < 7; i++) {
        float valve = computeValve(tests[i].level, tests[i].rate);
        Serial.println();
        Serial.print("[");
        Serial.print(tests[i].desc);
        Serial.print("]  Level=");
        Serial.print(tests[i].level, 1);
        Serial.print(" cm, Rate=");
        Serial.print(tests[i].rate, 1);
        Serial.print(" cm/s  -->  Valve = ");
        Serial.print(valve, 1);
        Serial.println(" %");
    }

    Serial.println("\nDone.");
}

void loop() { }
