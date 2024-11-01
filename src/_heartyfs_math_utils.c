#include "_heartyfs_math_utils.h"

int _maxInt(int a, int b) { return (a > b) ? a : b; }

int _minInt(int a, int b) { return (a < b) ? a : b; }

int _ceilDivInt(int n, int d) { return (n % d == 0) ? n / d : n / d + 1; }

int _absInt(int x) { return (x < 0) ? -x : x; }