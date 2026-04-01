/**
 * FIS_MembershipFunctions.cpp
 * ───────────────────────────
 * Implementation of membership-function shapes.
 * Direct port of membership.py (Python FIS).
 */

#include "FIS_MembershipFunctions.h"

/* ─── Free-standing membership functions ─────────────────────────── */

float fis_trimf(float x, float a, float b, float c) {
    if (a == b && b == c) {
        return (x == a) ? 1.0f : 0.0f;
    }
    float left  = (b != a) ? (x - a) / (b - a) : ((x >= b) ? 1.0f : 0.0f);
    float right = (c != b) ? (c - x) / (c - b) : ((x <= b) ? 1.0f : 0.0f);
    float val = left < right ? left : right;  // min
    return (val > 0.0f) ? val : 0.0f;         // max(0, val)
}

float fis_trapmf(float x, float a, float b, float c, float d) {
    if (x <= a || x >= d) return 0.0f;
    if (b <= x && x <= c) return 1.0f;
    if (x < b) {
        return (b != a) ? (x - a) / (b - a) : 1.0f;
    }
    /* x > c */
    return (d != c) ? (d - x) / (d - c) : 1.0f;
}

float fis_gaussmf(float x, float mean, float sigma) {
    float t = (x - mean) / sigma;
    return expf(-0.5f * t * t);
}

/* ─── MembershipFunction struct ──────────────────────────────────── */

FIS_MembershipFunction::FIS_MembershipFunction()
    : type(FIS_MF_NONE)
{
    for (int i = 0; i < 5; i++) params[i] = 0.0f;
}

float FIS_MembershipFunction::evaluate(float x) const {
    switch (type) {
        case FIS_MF_TRIANGULAR:
            return fis_trimf(x, params[0], params[1], params[2]);
        case FIS_MF_TRAPEZOIDAL:
            return fis_trapmf(x, params[0], params[1], params[2], params[3]);
        case FIS_MF_GAUSSIAN:
            return fis_gaussmf(x, params[0], params[1]);
        default:
            return 0.0f;
    }
}
