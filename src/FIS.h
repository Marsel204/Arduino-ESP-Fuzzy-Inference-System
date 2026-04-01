/**
 * FIS.h — Fuzzy Inference System for Arduino / ESP32
 * ───────────────────────────────────────────────────
 * Single-include header that pulls in every module of the FIS library.
 *
 * Usage:
 *   #include <FIS.h>
 *
 * To override compile-time limits, define them BEFORE including this header:
 *   #define FIS_MAX_VARIABLES 12
 *   #define FIS_MAX_RULES     64
 *   #include <FIS.h>
 *
 * Modules:
 *   1. Membership Functions  — fis_trimf, fis_trapmf, fis_gaussmf
 *   2. Fuzzification         — FIS_LinguisticVariable, FIS_FuzzificationModule
 *   3. Inference             — FIS_Rule, FIS_InferenceModule
 *   4. Defuzzification       — FIS_DefuzzificationModule
 *
 * Based on: https://github.com/Marsel204/Fuzzy-Inference-System
 */

#ifndef FIS_H
#define FIS_H

#include "FIS_Config.h"
#include "FIS_MembershipFunctions.h"
#include "FIS_Fuzzification.h"
#include "FIS_Inference.h"
#include "FIS_Defuzzification.h"

#endif /* FIS_H */
