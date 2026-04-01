/**
 * MotorSpeedPID.ino
 * ─────────────────
 * Fuzzy PD-like controller for DC motor speed regulation.
 *
 * Instead of tuning Kp and Kd gains, this example uses fuzzy logic
 * to map error and error-derivative into a control output, similar
 * to a PD controller but with non-linear, rule-based behaviour.
 *
 * Inputs:
 *   - error         = setpoint - actual_speed   [-100, +100] RPM
 *   - delta_error   = error(k) - error(k-1)    [-50, +50]   RPM/cycle
 *
 * Output:
 *   - control       = PWM adjustment            [-255, +255]
 *
 * Wiring:  Simulated — results to Serial at 115200 baud.
 *          In a real system, apply `control` as PWM delta to a motor driver.
 */

#include <FIS.h>

/* ── Linguistic variables ─────────────────────────────────────────── */
FIS_LinguisticVariable errVar;
FIS_LinguisticVariable derrVar;
FIS_LinguisticVariable ctrlVar;

/* ── FIS modules ──────────────────────────────────────────────────── */
FIS_FuzzificationModule   fuzzifier;
FIS_InferenceModule       engine;
FIS_DefuzzificationModule defuzzifier;

int idxErr, idxDerr;

void buildFIS() {

    /* ── Input 1: Error [-100, 100] RPM ───────────────────────────── */
    errVar.init("error", -100.0f, 100.0f);
    errVar.addTermTrapezoidal("negBig",   -101, -100, -60, -30);
    errVar.addTermTriangular( "negSmall",  -50,  -20,   0);
    errVar.addTermTriangular( "zero",      -15,    0,  15);
    errVar.addTermTriangular( "posSmall",    0,   20,  50);
    errVar.addTermTrapezoidal("posBig",     30,   60, 100, 101);

    /* ── Input 2: Delta Error [-50, 50] RPM/cycle ─────────────────── */
    derrVar.init("derror", -50.0f, 50.0f);
    derrVar.addTermTrapezoidal("negBig",  -51, -50, -25, -10);
    derrVar.addTermTriangular( "neg",     -20, -10,   0);
    derrVar.addTermTriangular( "zero",     -8,   0,   8);
    derrVar.addTermTriangular( "pos",       0,  10,  20);
    derrVar.addTermTrapezoidal("posBig",   10,  25,  50,  51);

    /* ── Output: Control [-255, 255] ──────────────────────────────── */
    ctrlVar.init("ctrl", -255.0f, 255.0f);
    ctrlVar.addTermTriangular("negBig",  -255, -255, -128);
    ctrlVar.addTermTriangular("negSmall",-180,  -90,    0);
    ctrlVar.addTermTriangular("zero",     -64,    0,   64);
    ctrlVar.addTermTriangular("posSmall",   0,   90,  180);
    ctrlVar.addTermTriangular("posBig",   128,  255,  255);

    /* ── Register ─────────────────────────────────────────────────── */
    idxErr  = fuzzifier.addVariable(&errVar);
    idxDerr = fuzzifier.addVariable(&derrVar);
    defuzzifier.addOutputVariable(&ctrlVar);

    /* ── Term indices ─────────────────────────────────────────────── */
    int eNB = errVar.findTerm("negBig");
    int eNS = errVar.findTerm("negSmall");
    int eZE = errVar.findTerm("zero");
    int ePS = errVar.findTerm("posSmall");
    int ePB = errVar.findTerm("posBig");

    int dNB = derrVar.findTerm("negBig");
    int dN  = derrVar.findTerm("neg");
    int dZE = derrVar.findTerm("zero");
    int dP  = derrVar.findTerm("pos");
    int dPB = derrVar.findTerm("posBig");

    int cNB = ctrlVar.findTerm("negBig");
    int cNS = ctrlVar.findTerm("negSmall");
    int cZE = ctrlVar.findTerm("zero");
    int cPS = ctrlVar.findTerm("posSmall");
    int cPB = ctrlVar.findTerm("posBig");

    /* ── Rule table (5x5 = 25 rules) ─────────────────────────────── */
    /* Rows: error (NB, NS, ZE, PS, PB)                               */
    /* Cols: delta-error (NB, N, ZE, P, PB)                            */
    int errTerms[]  = { eNB, eNS, eZE, ePS, ePB };
    int derrTerms[] = { dNB, dN,  dZE, dP,  dPB };

    /* Typical fuzzy-PD rule table: control = f(error, derror) */
    int ruleTable[5][5] = {
        /* dErr→  NB    N     ZE    P     PB   */
        /* eNB */ { cNB, cNB, cNB, cNS, cZE },
        /* eNS */ { cNB, cNB, cNS, cZE, cPS },
        /* eZE */ { cNB, cNS, cZE, cPS, cPB },
        /* ePS */ { cNS, cZE, cPS, cPB, cPB },
        /* ePB */ { cZE, cPS, cPB, cPB, cPB },
    };

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            FIS_Rule r;
            r.addAntecedent(idxErr,  errTerms[i]);
            r.addAntecedent(idxDerr, derrTerms[j]);
            r.setConsequent(0, ruleTable[i][j]);
            engine.addRule(r);
        }
    }
}

float computeControl(float error, float deltaError) {
    float inputs[2] = { error, deltaError };
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
    Serial.println("  FIS Library - Fuzzy PD Motor Controller");
    Serial.println("=============================================");
    Serial.print("Number of rules: ");

    buildFIS();

    Serial.println(engine.ruleCount());
    Serial.println();

    /* Simulate various error/delta-error scenarios */
    struct { float e; float de; const char* desc; } tests[] = {
        {  80.0f,   0.0f, "Large positive error, steady"     },
        {  20.0f, -10.0f, "Small pos error, improving"       },
        {   0.0f,   0.0f, "On target, stable"                },
        { -15.0f,   5.0f, "Small neg error, getting worse"   },
        { -70.0f,   0.0f, "Large negative error, steady"     },
        {  40.0f,  20.0f, "Medium error, diverging fast"     },
        {   5.0f,  -5.0f, "Nearly on target, converging"     },
    };

    for (int i = 0; i < 7; i++) {
        float ctrl = computeControl(tests[i].e, tests[i].de);
        Serial.print("[");
        Serial.print(tests[i].desc);
        Serial.print("]  err=");
        Serial.print(tests[i].e, 1);
        Serial.print(", derr=");
        Serial.print(tests[i].de, 1);
        Serial.print("  -->  ctrl = ");
        Serial.println(ctrl, 1);
    }

    Serial.println("\nDone.");
}

void loop() { }
