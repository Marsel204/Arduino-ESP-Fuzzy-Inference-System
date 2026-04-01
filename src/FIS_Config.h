/**
 * FIS_Config.h
 * ────────────
 * Compile-time capacity constants for the Fuzzy Inference System library.
 *
 * Override any of these BEFORE including <FIS.h> if you need larger or
 * smaller limits.  Example:
 *
 *   #define FIS_MAX_VARIABLES 12
 *   #define FIS_MAX_RULES     64
 *   #include <FIS.h>
 */

#ifndef FIS_CONFIG_H
#define FIS_CONFIG_H

/* Maximum number of linguistic variables (inputs + outputs combined) */
#ifndef FIS_MAX_VARIABLES
#define FIS_MAX_VARIABLES 8
#endif

/* Maximum number of fuzzy terms per linguistic variable */
#ifndef FIS_MAX_TERMS
#define FIS_MAX_TERMS 7
#endif

/* Maximum number of rules in the inference engine */
#ifndef FIS_MAX_RULES
#define FIS_MAX_RULES 32
#endif

/* Maximum number of antecedents (IF-clauses) per rule */
#ifndef FIS_MAX_ANTECEDENTS
#define FIS_MAX_ANTECEDENTS 4
#endif

/* Default number of sample points for centroid defuzzification */
#ifndef FIS_DEFUZZ_SAMPLES
#define FIS_DEFUZZ_SAMPLES 100
#endif

/* Maximum label length (including null terminator) for variable / term names */
#ifndef FIS_MAX_LABEL_LEN
#define FIS_MAX_LABEL_LEN 16
#endif

#endif /* FIS_CONFIG_H */
