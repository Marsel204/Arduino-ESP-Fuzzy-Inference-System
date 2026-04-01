/**
 * FIS_MembershipFunctions.h
 * ─────────────────────────
 * Reusable membership-function shapes for fuzzy sets.
 *
 * Each function returns a float in [0, 1].
 * Direct port of membership.py from the Python FIS.
 */

#ifndef FIS_MEMBERSHIP_FUNCTIONS_H
#define FIS_MEMBERSHIP_FUNCTIONS_H

#include <math.h>

/**
 * Enumeration of supported membership-function types.
 * Used internally by MembershipFunction to evaluate without lambdas.
 */
enum FIS_MFType {
    FIS_MF_NONE = 0,
    FIS_MF_TRIANGULAR,    ///< params: a, b, c
    FIS_MF_TRAPEZOIDAL,   ///< params: a, b, c, d
    FIS_MF_GAUSSIAN       ///< params: mean, sigma
};

/**
 * Triangular membership function.
 *
 * @param x  Crisp input value
 * @param a  Left foot  (μ = 0)
 * @param b  Peak       (μ = 1)
 * @param c  Right foot (μ = 0)
 * @return   Degree of membership in [0, 1]
 */
float fis_trimf(float x, float a, float b, float c);

/**
 * Trapezoidal membership function.
 *
 * @param x  Crisp input value
 * @param a  Left outer foot   (μ = 0)
 * @param b  Left inner shoulder (μ = 1)
 * @param c  Right inner shoulder (μ = 1)
 * @param d  Right outer foot  (μ = 0)
 * @return   Degree of membership in [0, 1]
 */
float fis_trapmf(float x, float a, float b, float c, float d);

/**
 * Gaussian membership function.
 *
 * @param x      Crisp input value
 * @param mean   Centre of the bell
 * @param sigma  Width parameter (standard deviation)
 * @return       Degree of membership in [0, 1]
 */
float fis_gaussmf(float x, float mean, float sigma);

/**
 * Compact struct holding an MF type and its parameters.
 * Replaces Python lambdas — evaluate() dispatches to the correct function.
 */
struct FIS_MembershipFunction {
    FIS_MFType type;
    float params[5]; ///< Up to 5 params (trapmf uses 4, gaussmf uses 2, etc.)

    FIS_MembershipFunction();

    /**
     * Evaluate the membership function at point x.
     * Dispatches based on stored `type`.
     */
    float evaluate(float x) const;
};

#endif /* FIS_MEMBERSHIP_FUNCTIONS_H */
