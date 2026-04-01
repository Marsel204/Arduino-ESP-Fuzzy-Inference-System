/**
 * FIS_Defuzzification.cpp
 * ───────────────────────
 * Implementation of DefuzzificationModule.
 * Port of defuzzification.py – Centroid (Center of Gravity) method.
 */

#include "FIS_Defuzzification.h"

/* ─── FIS_DefuzzificationModule ──────────────────────────────────── */

FIS_DefuzzificationModule::FIS_DefuzzificationModule()
    : _count(0)
{
    for (int i = 0; i < FIS_MAX_VARIABLES; i++) _vars[i] = nullptr;
}

int FIS_DefuzzificationModule::addOutputVariable(FIS_LinguisticVariable* var) {
    if (_count >= FIS_MAX_VARIABLES) return -1;
    _vars[_count] = var;
    return _count++;
}

int FIS_DefuzzificationModule::findOutputVariable(const char* name) const {
    for (int i = 0; i < _count; i++) {
        if (strcmp(_vars[i]->name, name) == 0) return i;
    }
    return -1;
}

FIS_LinguisticVariable* FIS_DefuzzificationModule::getOutputVariable(int index) const {
    if (index < 0 || index >= _count) return nullptr;
    return _vars[index];
}

void FIS_DefuzzificationModule::defuzzify(const float strengths[][FIS_MAX_TERMS],
                                           float* crispOutputs,
                                           int numSamples) const {
    for (int v = 0; v < _count; v++) {
        FIS_LinguisticVariable* var = _vars[v];

        /* Step size across the universe of discourse */
        float step = (var->hi - var->lo) / (float)(numSamples - 1);

        float areaSum = 0.0f;       /* Σ y_i           */
        float centroidSum = 0.0f;   /* Σ (x_i * y_i)   */

        for (int i = 0; i < numSamples; i++) {
            float x = var->lo + (float)i * step;
            float y = 0.0f;

            /* For each term, clip MF by its firing strength and take max */
            for (int t = 0; t < var->termCount; t++) {
                float strength = strengths[v][t];
                if (strength <= 0.0f) continue;

                float mfVal = var->evaluate(t, x);
                /* Mamdani clipping: min(strength, mf(x)) */
                float clipped = (strength < mfVal) ? strength : mfVal;
                /* Aggregate (union): max */
                if (clipped > y) y = clipped;
            }

            areaSum += y;
            centroidSum += x * y;
        }

        if (areaSum == 0.0f) {
            /* Fallback: midpoint of universe (no rules fired) */
            crispOutputs[v] = (var->lo + var->hi) / 2.0f;
        } else {
            crispOutputs[v] = centroidSum / areaSum;
        }
    }
}

int FIS_DefuzzificationModule::outputVariableCount() const {
    return _count;
}
