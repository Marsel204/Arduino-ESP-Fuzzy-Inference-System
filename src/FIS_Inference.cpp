/**
 * FIS_Inference.cpp
 * ─────────────────
 * Implementation of Rule and InferenceModule.
 * Port of inference.py – Mamdani max-min inference.
 */

#include "FIS_Inference.h"

/* ─── FIS_Antecedent ─────────────────────────────────────────────── */

FIS_Antecedent::FIS_Antecedent()
    : variableIndex(-1), termIndex(-1) {}

FIS_Antecedent::FIS_Antecedent(int varIdx, int termIdx)
    : variableIndex(varIdx), termIndex(termIdx) {}

/* ─── FIS_Rule ───────────────────────────────────────────────────── */

FIS_Rule::FIS_Rule()
    : antecedentCount(0),
      consequentVarIndex(-1),
      consequentTermIndex(-1) {}

int FIS_Rule::addAntecedent(int varIndex, int termIndex) {
    if (antecedentCount >= FIS_MAX_ANTECEDENTS) return -1;
    antecedents[antecedentCount].variableIndex = varIndex;
    antecedents[antecedentCount].termIndex = termIndex;
    antecedentCount++;
    return antecedentCount;
}

void FIS_Rule::setConsequent(int varIndex, int termIndex) {
    consequentVarIndex = varIndex;
    consequentTermIndex = termIndex;
}

float FIS_Rule::evaluate(const float inputDegrees[][FIS_MAX_TERMS]) const {
    if (antecedentCount == 0) return 0.0f;

    float firingStrength = 1.0f; /* Identity for min */
    for (int i = 0; i < antecedentCount; i++) {
        int vi = antecedents[i].variableIndex;
        int ti = antecedents[i].termIndex;
        if (vi < 0 || ti < 0) return 0.0f;

        float degree = inputDegrees[vi][ti];
        if (degree < firingStrength) {
            firingStrength = degree;  /* AND = min */
        }
    }
    return firingStrength;
}

/* ─── FIS_InferenceModule ────────────────────────────────────────── */

FIS_InferenceModule::FIS_InferenceModule()
    : _count(0) {}

int FIS_InferenceModule::addRule(const FIS_Rule& rule) {
    if (_count >= FIS_MAX_RULES) return -1;
    _rules[_count] = rule;
    return _count++;
}

void FIS_InferenceModule::infer(const float inputDegrees[][FIS_MAX_TERMS],
                                 float outStrengths[][FIS_MAX_TERMS],
                                 int numOutputVars) const {
    /* Zero the output array */
    for (int v = 0; v < numOutputVars; v++) {
        for (int t = 0; t < FIS_MAX_TERMS; t++) {
            outStrengths[v][t] = 0.0f;
        }
    }

    /* Evaluate each rule and aggregate with max() */
    for (int r = 0; r < _count; r++) {
        float strength = _rules[r].evaluate(inputDegrees);
        int ov = _rules[r].consequentVarIndex;
        int ot = _rules[r].consequentTermIndex;

        if (ov < 0 || ov >= numOutputVars) continue;
        if (ot < 0 || ot >= FIS_MAX_TERMS) continue;

        if (strength > outStrengths[ov][ot]) {
            outStrengths[ov][ot] = strength; /* OR / aggregation = max */
        }
    }
}

int FIS_InferenceModule::ruleCount() const {
    return _count;
}
