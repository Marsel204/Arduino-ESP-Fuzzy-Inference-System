/**
 * TippingProblem.ino
 * ──────────────────
 * Classic "Tipping Problem" example for the FIS library.
 *
 * This is a direct port of main.py from the Python FIS.
 *
 * Inputs:  food quality [0-10], service level [0-10]
 * Output:  tip percentage [0-30%]
 *
 * Wiring:  None — output goes to Serial Monitor at 115200 baud.
 */

#include <FIS.h>

/* ──────────────────────────────────────────────────────────────────── */
/*  Global FIS objects                                                  */
/* ──────────────────────────────────────────────────────────────────── */

/* Input variables */
FIS_LinguisticVariable foodVar;
FIS_LinguisticVariable serviceVar;

/* Output variable */
FIS_LinguisticVariable tipVar;

/* Modules */
FIS_FuzzificationModule   fuzzifier;
FIS_InferenceModule       inferenceEngine;
FIS_DefuzzificationModule defuzzifier;

/* Variable indices (assigned during setup) */
int idxFood, idxService;   /* in fuzzifier   */
int idxTipOut;              /* in defuzzifier */

/* ──────────────────────────────────────────────────────────────────── */
/*  Build the FIS (mirrors Python build_fis())                         */
/* ──────────────────────────────────────────────────────────────────── */
void buildFIS() {

    /* ── 1. Define linguistic variables & their fuzzy terms ────────── */

    /* Input 1: Food Quality [0, 10] */
    foodVar.init("food", 0.0f, 10.0f);
    foodVar.addTermTrapezoidal("poor",      -1, 0, 1, 3);
    foodVar.addTermTrapezoidal("average",    2, 4, 6, 8);
    foodVar.addTermTrapezoidal("excellent",  7, 9, 10, 11);

    /* Input 2: Service Level [0, 10] */
    serviceVar.init("service", 0.0f, 10.0f);
    serviceVar.addTermTrapezoidal("poor",      -1, 0, 1, 4);
    serviceVar.addTermTrapezoidal("good",       3, 5, 5, 7);
    serviceVar.addTermTrapezoidal("excellent",  6, 9, 10, 11);

    /* Output: Tip Percentage [0, 30] */
    tipVar.init("tip", 0.0f, 30.0f);
    tipVar.addTermTriangular("low",     0,  5, 10);
    tipVar.addTermTriangular("medium", 10, 15, 20);
    tipVar.addTermTriangular("high",   20, 25, 30);

    /* ── 2. Module 1: Fuzzification ───────────────────────────────── */
    idxFood    = fuzzifier.addVariable(&foodVar);
    idxService = fuzzifier.addVariable(&serviceVar);

    /* ── 3. Module 2: Inference ───────────────────────────────────── */

    /* Look up term indices for readability */
    int tFoodPoor      = foodVar.findTerm("poor");
    int tFoodAverage   = foodVar.findTerm("average");
    int tFoodExcellent = foodVar.findTerm("excellent");

    int tServPoor      = serviceVar.findTerm("poor");
    int tServGood      = serviceVar.findTerm("good");
    int tServExcellent = serviceVar.findTerm("excellent");

    int tTipLow    = tipVar.findTerm("low");
    int tTipMedium = tipVar.findTerm("medium");
    int tTipHigh   = tipVar.findTerm("high");

    /* Output variable index in the defuzzifier */
    int outTipIdx = 0;

    /* Rule 1a: IF food IS poor THEN tip IS low */
    FIS_Rule r1a;
    r1a.addAntecedent(idxFood, tFoodPoor);
    r1a.setConsequent(outTipIdx, tTipLow);
    inferenceEngine.addRule(r1a);

    /* Rule 1b: IF service IS poor THEN tip IS low */
    FIS_Rule r1b;
    r1b.addAntecedent(idxService, tServPoor);
    r1b.setConsequent(outTipIdx, tTipLow);
    inferenceEngine.addRule(r1b);

    /* Rule 2: IF service IS good THEN tip IS medium */
    FIS_Rule r2;
    r2.addAntecedent(idxService, tServGood);
    r2.setConsequent(outTipIdx, tTipMedium);
    inferenceEngine.addRule(r2);

    /* Rule 3a: IF food IS excellent THEN tip IS high */
    FIS_Rule r3a;
    r3a.addAntecedent(idxFood, tFoodExcellent);
    r3a.setConsequent(outTipIdx, tTipHigh);
    inferenceEngine.addRule(r3a);

    /* Rule 3b: IF service IS excellent THEN tip IS high */
    FIS_Rule r3b;
    r3b.addAntecedent(idxService, tServExcellent);
    r3b.setConsequent(outTipIdx, tTipHigh);
    inferenceEngine.addRule(r3b);

    /* Rule 4: IF food IS average AND service IS good THEN tip IS medium */
    FIS_Rule r4;
    r4.addAntecedent(idxFood, tFoodAverage);
    r4.addAntecedent(idxService, tServGood);
    r4.setConsequent(outTipIdx, tTipMedium);
    inferenceEngine.addRule(r4);

    /* ── 4. Module 3: Defuzzification ─────────────────────────────── */
    idxTipOut = defuzzifier.addOutputVariable(&tipVar);
}

/* ──────────────────────────────────────────────────────────────────── */
/*  Run a test (mirrors Python run_test())                             */
/* ──────────────────────────────────────────────────────────────────── */
float runTest(float foodScore, float serviceScore) {

    Serial.println();
    Serial.print("--- Testing FIS: Food=");
    Serial.print(foodScore, 1);
    Serial.print(", Service=");
    Serial.print(serviceScore, 1);
    Serial.println(" ---");

    /* Step 1: Fuzzification */
    float crispInputs[2] = { foodScore, serviceScore };
    float fuzzyDegrees[FIS_MAX_VARIABLES][FIS_MAX_TERMS];
    fuzzifier.fuzzify(crispInputs, fuzzyDegrees);

    Serial.println("1. Fuzzified Inputs:");
    for (int v = 0; v < fuzzifier.variableCount(); v++) {
        FIS_LinguisticVariable* var = fuzzifier.getVariable(v);
        Serial.print("   ");
        Serial.print(var->name);
        Serial.print(": ");
        for (int t = 0; t < var->termCount; t++) {
            Serial.print(var->terms[t].label);
            Serial.print("=");
            Serial.print(fuzzyDegrees[v][t], 4);
            if (t < var->termCount - 1) Serial.print(", ");
        }
        Serial.println();
    }

    /* Step 2: Inference */
    int numOuts = defuzzifier.outputVariableCount();
    float outStrengths[FIS_MAX_VARIABLES][FIS_MAX_TERMS];
    inferenceEngine.infer(fuzzyDegrees, outStrengths, numOuts);

    Serial.println("2. Inferred Fuzzy Outputs (Aggregated):");
    for (int v = 0; v < numOuts; v++) {
        FIS_LinguisticVariable* var = defuzzifier.getOutputVariable(v);
        Serial.print("   ");
        Serial.print(var->name);
        Serial.print(": ");
        for (int t = 0; t < var->termCount; t++) {
            Serial.print(var->terms[t].label);
            Serial.print("=");
            Serial.print(outStrengths[v][t], 4);
            if (t < var->termCount - 1) Serial.print(", ");
        }
        Serial.println();
    }

    /* Step 3: Defuzzification */
    float crispOutputs[FIS_MAX_VARIABLES];
    defuzzifier.defuzzify(outStrengths, crispOutputs);

    Serial.print("3. Defuzzified Crisp Output: Tip Percentage = ");
    Serial.print(crispOutputs[idxTipOut], 2);
    Serial.println("%");

    return crispOutputs[idxTipOut];
}

/* ──────────────────────────────────────────────────────────────────── */
/*  Arduino entry points                                               */
/* ──────────────────────────────────────────────────────────────────── */

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }

    Serial.println("========================================");
    Serial.println("  FIS Library - Tipping Problem Example");
    Serial.println("========================================");

    buildFIS();

    /* Three test cases matching main.py */
    runTest(8.5f, 9.0f);   /* Expected: high tip  (~25%) */
    runTest(2.0f, 3.0f);   /* Expected: low tip   (~5%)  */
    runTest(5.0f, 5.0f);   /* Expected: medium tip (~15%) */

    Serial.println();
    Serial.println("FIS Pipeline executed successfully.");
}

void loop() {
    /* Nothing to do — one-shot demo */
}
