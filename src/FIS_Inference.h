/**
 * FIS_Inference.h
 * ───────────────
 * Module 2 – Inference
 *
 * Port of inference.py from the Python FIS.
 *
 * Responsibilities:
 * - Store fuzzy rules.
 * - Evaluate rules against fuzzified inputs (Mamdani max-min).
 * - Aggregate outputs: AND via min(), rule aggregation via max().
 */

#ifndef FIS_INFERENCE_H
#define FIS_INFERENCE_H

#include "FIS_Config.h"

/**
 * An antecedent clause:  "variable IS term"
 * Stored as integer indices for fast runtime lookup.
 */
struct FIS_Antecedent {
    int variableIndex;  ///< Index of the input variable (in FuzzificationModule)
    int termIndex;      ///< Index of the term within that variable

    FIS_Antecedent();
    FIS_Antecedent(int varIdx, int termIdx);
};

/**
 * A single fuzzy rule (Mamdani style).
 *
 * IF var1 IS term1 AND var2 IS term2 ... THEN outVar IS outTerm
 *
 * Antecedents are combined with AND (min).
 * To express OR, add multiple rules with the same consequent.
 */
class FIS_Rule {
public:
    FIS_Antecedent antecedents[FIS_MAX_ANTECEDENTS];
    int antecedentCount;

    int consequentVarIndex;   ///< Index of the output variable
    int consequentTermIndex;  ///< Index of the output term

    FIS_Rule();

    /**
     * Add an antecedent clause to this rule.
     * @param varIndex   Input variable index
     * @param termIndex  Term index within that variable
     * @return           Number of antecedents after adding, or -1 on overflow
     */
    int addAntecedent(int varIndex, int termIndex);

    /**
     * Set the consequent of this rule.
     * @param varIndex   Output variable index (in DefuzzificationModule)
     * @param termIndex  Term index within that output variable
     */
    void setConsequent(int varIndex, int termIndex);

    /**
     * Evaluate the firing strength of this rule.
     *
     * @param inputDegrees  2D array [variableIndex][termIndex] of membership degrees
     * @return              Firing strength (min of all antecedent degrees)
     */
    float evaluate(const float inputDegrees[][FIS_MAX_TERMS]) const;
};


/**
 * Module 2 — Inference Module.
 *
 * Stores rules and performs Mamdani max-min inference.
 */
class FIS_InferenceModule {
public:
    FIS_InferenceModule();

    /**
     * Add a rule to the engine.
     * @param rule  The rule to add (copied by value).
     * @return      Index of the added rule, or -1 on overflow.
     */
    int addRule(const FIS_Rule& rule);

    /**
     * Infer aggregated output strengths from fuzzified inputs.
     *
     * @param inputDegrees    2D array [inputVarIndex][termIndex]
     * @param outStrengths    2D array [outputVarIndex][termIndex] filled with
     *                        aggregated firing strengths (max of all rules
     *                        mapping to the same consequent).
     * @param numOutputVars   Number of output variables
     *
     * NOTE: outStrengths should be zeroed before calling this method.
     */
    void infer(const float inputDegrees[][FIS_MAX_TERMS],
               float outStrengths[][FIS_MAX_TERMS],
               int numOutputVars) const;

    /** Number of registered rules. */
    int ruleCount() const;

private:
    FIS_Rule _rules[FIS_MAX_RULES];
    int _count;
};

#endif /* FIS_INFERENCE_H */
