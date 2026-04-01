/**
 * FIS_Defuzzification.h
 * ─────────────────────
 * Module 3 – Defuzzification
 *
 * Port of defuzzification.py from the Python FIS.
 *
 * Responsibilities:
 * - Given aggregated firing strengths and the output LinguisticVariable
 *   definitions, compute crisp values using the Centroid (CoG) method.
 */

#ifndef FIS_DEFUZZIFICATION_H
#define FIS_DEFUZZIFICATION_H

#include "FIS_Config.h"
#include "FIS_Fuzzification.h"

/**
 * Module 3 — Defuzzification Module.
 *
 * Converts aggregated fuzzy outputs into crisp numerical values
 * using the Centroid (Center of Gravity) method.
 */
class FIS_DefuzzificationModule {
public:
    FIS_DefuzzificationModule();

    /**
     * Register an output linguistic variable.
     * @param var  Pointer to the variable (must remain in scope).
     * @return     Index assigned, or -1 on overflow.
     */
    int addOutputVariable(FIS_LinguisticVariable* var);

    /**
     * Find a registered output variable by name.
     * @return Index, or -1 if not found.
     */
    int findOutputVariable(const char* name) const;

    /**
     * Get a pointer to a registered output variable by index.
     */
    FIS_LinguisticVariable* getOutputVariable(int index) const;

    /**
     * Defuzzify the aggregated fuzzy outputs to produce crisp values.
     *
     * Uses Centroid (Center of Gravity) with numerical integration.
     *
     * @param strengths     2D array [outputVarIndex][termIndex] of aggregated
     *                      firing strengths (from InferenceModule::infer).
     * @param crispOutputs  Array filled with one crisp value per output variable.
     * @param numSamples    Number of sample points for numerical integration
     *                      (default = FIS_DEFUZZ_SAMPLES).
     */
    void defuzzify(const float strengths[][FIS_MAX_TERMS],
                   float* crispOutputs,
                   int numSamples = FIS_DEFUZZ_SAMPLES) const;

    /** Number of registered output variables. */
    int outputVariableCount() const;

private:
    FIS_LinguisticVariable* _vars[FIS_MAX_VARIABLES];
    int _count;
};

#endif /* FIS_DEFUZZIFICATION_H */
