# FIS — Fuzzy Inference System Library for Arduino / ESP32

A lightweight, zero-dependency C++ library implementing a complete **Mamdani-style Fuzzy Inference System** pipeline for Arduino and ESP32 microcontrollers.

Ported from [Marsel204/Fuzzy-Inference-System](https://github.com/Marsel204/Fuzzy-Inference-System) (Python).

## Features

- **Three membership function types**: Triangular, Trapezoidal, Gaussian
- **Complete FIS pipeline**: Fuzzification → Inference (max-min) → Defuzzification (Centroid)
- **No dynamic memory allocation** — uses fixed-size arrays, safe for embedded targets
- **No external dependencies** — only `<math.h>` and `<string.h>`
- **Configurable limits** — override `FIS_MAX_VARIABLES`, `FIS_MAX_RULES`, etc. at compile time
- **Multi-input, multi-output** support

## Installation

### Arduino IDE
1. Copy this folder into your Arduino `libraries/` directory (e.g. `~/Arduino/libraries/FIS Library/`)
2. Restart the Arduino IDE
3. Go to **Sketch → Include Library → FIS**

### PlatformIO
Add to your `platformio.ini`:
```ini
lib_deps =
    /path/to/FIS Library
```

## Quick Start

```cpp
#include <FIS.h>

// 1. Define variables
FIS_LinguisticVariable food, tip;

void setup() {
  Serial.begin(115200);

  // Input variable: food quality [0, 10]
  food.init("food", 0, 10);
  food.addTermTriangular("poor", 0, 0, 5);
  food.addTermTriangular("good", 5, 10, 10);

  // Output variable: tip percentage [0, 30]
  tip.init("tip", 0, 30);
  tip.addTermTriangular("low",  0,  0, 15);
  tip.addTermTriangular("high", 15, 30, 30);

  // 2. Fuzzification
  FIS_FuzzificationModule fuzzifier;
  int idxFood = fuzzifier.addVariable(&food);

  // 3. Inference
  FIS_InferenceModule engine;

  FIS_Rule r1;
  r1.addAntecedent(idxFood, food.findTerm("good"));
  r1.setConsequent(0, tip.findTerm("high"));
  engine.addRule(r1);

  // 4. Defuzzification
  FIS_DefuzzificationModule defuzz;
  defuzz.addOutputVariable(&tip);

  // Run the pipeline
  float inputs[] = { 8.0f };
  float degrees[FIS_MAX_VARIABLES][FIS_MAX_TERMS];
  fuzzifier.fuzzify(inputs, degrees);

  float outStrengths[FIS_MAX_VARIABLES][FIS_MAX_TERMS];
  engine.infer(degrees, outStrengths, 1);

  float crispOutput[1];
  defuzz.defuzzify(outStrengths, crispOutput);

  Serial.print("Tip: ");
  Serial.println(crispOutput[0]);
}

void loop() {}
```

## Architecture

The library mirrors the Python FIS architecture:

| Module | Class | Python Equivalent |
|--------|-------|-------------------|
| Membership Functions | `fis_trimf()`, `fis_trapmf()`, `fis_gaussmf()` | `membership.py` |
| Fuzzification | `FIS_LinguisticVariable`, `FIS_FuzzificationModule` | `fuzzification.py` |
| Inference | `FIS_Rule`, `FIS_InferenceModule` | `inference.py` |
| Defuzzification | `FIS_DefuzzificationModule` | `defuzzification.py` |

### Key Differences from the Python Version

| Aspect | Python | C++ (Arduino) |
|--------|--------|---------------|
| MF definition | Lambda functions | Enum type + parameter array |
| Variable/term refs | String dict keys | Integer indices (faster) |
| Memory | Dynamic dicts/lists | Fixed-size arrays |
| Floating point | `float`/`double` | `float` only (ESP32 FPU) |

## Compile-Time Configuration

Override these **before** `#include <FIS.h>`:

```cpp
#define FIS_MAX_VARIABLES   8   // Max input + output variables
#define FIS_MAX_TERMS       7   // Max terms per variable
#define FIS_MAX_RULES      32   // Max inference rules
#define FIS_MAX_ANTECEDENTS 4   // Max IF-clauses per rule
#define FIS_DEFUZZ_SAMPLES 100  // Centroid resolution
#define FIS_MAX_LABEL_LEN  16   // Max chars in a name/label

#include <FIS.h>
```

## API Reference

### Membership Functions

```cpp
float fis_trimf(float x, float a, float b, float c);
float fis_trapmf(float x, float a, float b, float c, float d);
float fis_gaussmf(float x, float mean, float sigma);
```

### FIS_LinguisticVariable

```cpp
void init(const char* name, float lo, float hi);
int  addTermTriangular(const char* label, float a, float b, float c);
int  addTermTrapezoidal(const char* label, float a, float b, float c, float d);
int  addTermGaussian(const char* label, float mean, float sigma);
int  findTerm(const char* label);
float evaluate(int termIndex, float x);
```

### FIS_FuzzificationModule

```cpp
int  addVariable(FIS_LinguisticVariable* var);
int  findVariable(const char* name);
void fuzzify(const float* crispInputs, float outDegrees[][FIS_MAX_TERMS]);
int  variableCount();
```

### FIS_Rule / FIS_InferenceModule

```cpp
// Rule
int  addAntecedent(int varIndex, int termIndex);
void setConsequent(int varIndex, int termIndex);

// Module
int  addRule(const FIS_Rule& rule);
void infer(const float inputDegrees[][FIS_MAX_TERMS],
           float outStrengths[][FIS_MAX_TERMS], int numOutputVars);
```

### FIS_DefuzzificationModule

```cpp
int  addOutputVariable(FIS_LinguisticVariable* var);
void defuzzify(const float strengths[][FIS_MAX_TERMS],
               float* crispOutputs, int numSamples = FIS_DEFUZZ_SAMPLES);
```

## License

MIT

## Credits

Ported from the Python [Fuzzy-Inference-System](https://github.com/Marsel204/Fuzzy-Inference-System) by Marsel204.
