/**
 * FIS_Fuzzification.cpp
 * ─────────────────────
 * Implementation of LinguisticVariable and FuzzificationModule.
 */

#include "FIS_Fuzzification.h"

/* ─── FIS_Term ───────────────────────────────────────────────────── */

FIS_Term::FIS_Term() {
    label[0] = '\0';
}

/* ─── FIS_LinguisticVariable ─────────────────────────────────────── */

FIS_LinguisticVariable::FIS_LinguisticVariable()
    : lo(0.0f), hi(0.0f), termCount(0)
{
    name[0] = '\0';
}

void FIS_LinguisticVariable::init(const char* n, float low, float high) {
    strncpy(name, n, FIS_MAX_LABEL_LEN - 1);
    name[FIS_MAX_LABEL_LEN - 1] = '\0';
    lo = low;
    hi = high;
    termCount = 0;
}

int FIS_LinguisticVariable::addTermTriangular(const char* label,
                                               float a, float b, float c) {
    if (termCount >= FIS_MAX_TERMS) return -1;
    int idx = termCount++;
    strncpy(terms[idx].label, label, FIS_MAX_LABEL_LEN - 1);
    terms[idx].label[FIS_MAX_LABEL_LEN - 1] = '\0';
    terms[idx].mf.type = FIS_MF_TRIANGULAR;
    terms[idx].mf.params[0] = a;
    terms[idx].mf.params[1] = b;
    terms[idx].mf.params[2] = c;
    return idx;
}

int FIS_LinguisticVariable::addTermTrapezoidal(const char* label,
                                                float a, float b,
                                                float c, float d) {
    if (termCount >= FIS_MAX_TERMS) return -1;
    int idx = termCount++;
    strncpy(terms[idx].label, label, FIS_MAX_LABEL_LEN - 1);
    terms[idx].label[FIS_MAX_LABEL_LEN - 1] = '\0';
    terms[idx].mf.type = FIS_MF_TRAPEZOIDAL;
    terms[idx].mf.params[0] = a;
    terms[idx].mf.params[1] = b;
    terms[idx].mf.params[2] = c;
    terms[idx].mf.params[3] = d;
    return idx;
}

int FIS_LinguisticVariable::addTermGaussian(const char* label,
                                             float mean, float sigma) {
    if (termCount >= FIS_MAX_TERMS) return -1;
    int idx = termCount++;
    strncpy(terms[idx].label, label, FIS_MAX_LABEL_LEN - 1);
    terms[idx].label[FIS_MAX_LABEL_LEN - 1] = '\0';
    terms[idx].mf.type = FIS_MF_GAUSSIAN;
    terms[idx].mf.params[0] = mean;
    terms[idx].mf.params[1] = sigma;
    return idx;
}

int FIS_LinguisticVariable::findTerm(const char* label) const {
    for (int i = 0; i < termCount; i++) {
        if (strcmp(terms[i].label, label) == 0) return i;
    }
    return -1;
}

float FIS_LinguisticVariable::evaluate(int termIndex, float x) const {
    if (termIndex < 0 || termIndex >= termCount) return 0.0f;
    return terms[termIndex].mf.evaluate(x);
}

/* ─── FIS_FuzzificationModule ────────────────────────────────────── */

FIS_FuzzificationModule::FIS_FuzzificationModule()
    : _count(0)
{
    for (int i = 0; i < FIS_MAX_VARIABLES; i++) _vars[i] = nullptr;
}

int FIS_FuzzificationModule::addVariable(FIS_LinguisticVariable* var) {
    if (_count >= FIS_MAX_VARIABLES) return -1;
    _vars[_count] = var;
    return _count++;
}

int FIS_FuzzificationModule::findVariable(const char* name) const {
    for (int i = 0; i < _count; i++) {
        if (strcmp(_vars[i]->name, name) == 0) return i;
    }
    return -1;
}

FIS_LinguisticVariable* FIS_FuzzificationModule::getVariable(int index) const {
    if (index < 0 || index >= _count) return nullptr;
    return _vars[index];
}

void FIS_FuzzificationModule::fuzzify(const float* crispInputs,
                                       float outDegrees[][FIS_MAX_TERMS]) const {
    for (int v = 0; v < _count; v++) {
        FIS_LinguisticVariable* var = _vars[v];
        for (int t = 0; t < var->termCount; t++) {
            outDegrees[v][t] = var->evaluate(t, crispInputs[v]);
        }
        /* Zero unused term slots */
        for (int t = var->termCount; t < FIS_MAX_TERMS; t++) {
            outDegrees[v][t] = 0.0f;
        }
    }
}

int FIS_FuzzificationModule::variableCount() const {
    return _count;
}
