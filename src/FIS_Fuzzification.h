/**
 * FIS_Fuzzification.h
 * ────────────────────
 * Module 1 – Fuzzification
 *
 * Port of fuzzification.py from the Python FIS.
 *
 * Responsibilities:
 * - Store linguistic variables, each with a universe of discourse and
 *   an array of term → membership function.
 * - Accept crisp inputs and compute membership degrees for every term.
 */

#ifndef FIS_FUZZIFICATION_H
#define FIS_FUZZIFICATION_H

#include "FIS_Config.h"
#include "FIS_MembershipFunctions.h"
#include <string.h>

/**
 * A single fuzzy term: a label paired with a membership function.
 */
struct FIS_Term {
    char label[FIS_MAX_LABEL_LEN];
    FIS_MembershipFunction mf;

    FIS_Term();
};

/**
 * Represents a single linguistic variable (input OR output).
 *
 * Stores its name, universe-of-discourse bounds [lo, hi],
 * and up to FIS_MAX_TERMS fuzzy terms.
 */
class FIS_LinguisticVariable {
public:
    char name[FIS_MAX_LABEL_LEN];
    float lo;
    float hi;
    FIS_Term terms[FIS_MAX_TERMS];
    int termCount;

    FIS_LinguisticVariable();

    /**
     * Initialise the variable.
     * @param name  Human-readable label (e.g. "food")
     * @param lo    Lower bound of universe of discourse
     * @param hi    Upper bound of universe of discourse
     */
    void init(const char* name, float lo, float hi);

    /**
     * Register a triangular fuzzy term.
     * @param label  Term name (e.g. "poor")
     * @param a      Left foot, b peak, c right foot
     * @return       Index of the added term, or -1 on overflow
     */
    int addTermTriangular(const char* label, float a, float b, float c);

    /**
     * Register a trapezoidal fuzzy term.
     * @param label  Term name
     * @param a,b,c,d  Trapezoid parameters
     * @return       Index of the added term, or -1 on overflow
     */
    int addTermTrapezoidal(const char* label, float a, float b, float c, float d);

    /**
     * Register a Gaussian fuzzy term.
     * @param label  Term name
     * @param mean   Centre of the bell
     * @param sigma  Width (std-dev)
     * @return       Index of the added term, or -1 on overflow
     */
    int addTermGaussian(const char* label, float mean, float sigma);

    /**
     * Find the index of a term by name.
     * @return Index (0-based), or -1 if not found.
     */
    int findTerm(const char* label) const;

    /**
     * Evaluate the membership degree for a given term at value x.
     */
    float evaluate(int termIndex, float x) const;
};


/**
 * Module 1 — Fuzzification Module.
 *
 * Holds pointers to registered linguistic variables and converts
 * crisp inputs into membership degrees.
 */
class FIS_FuzzificationModule {
public:
    FIS_FuzzificationModule();

    /**
     * Register a linguistic variable (input).
     * @param var  Pointer to the variable (must remain in scope)
     * @return     Index assigned to this variable, or -1 on overflow
     */
    int addVariable(FIS_LinguisticVariable* var);

    /**
     * Find a registered variable by name.
     * @return Index, or -1 if not found.
     */
    int findVariable(const char* name) const;

    /**
     * Get a pointer to a registered variable by index.
     */
    FIS_LinguisticVariable* getVariable(int index) const;

    /**
     * Fuzzify all registered variables.
     *
     * @param crispInputs  Array of crisp values, one per registered variable
     *                     (in registration order).
     * @param outDegrees   2D array [variableIndex][termIndex] filled with
     *                     membership degrees.
     */
    void fuzzify(const float* crispInputs,
                 float outDegrees[][FIS_MAX_TERMS]) const;

    /** Number of registered variables. */
    int variableCount() const;

private:
    FIS_LinguisticVariable* _vars[FIS_MAX_VARIABLES];
    int _count;
};

#endif /* FIS_FUZZIFICATION_H */
