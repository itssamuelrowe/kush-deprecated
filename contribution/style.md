# Style Guidelines

## White Space

### Blank Lines

Blank lines improve readability by seperating sections of code.

A blank line should be used in the following cases:
 * Between structures, functions, typedef declarations
 * Between logical sections inside a function

### Spaces

Spaces should be used in the following circumstances:

 * A keyword followed by a parenthesis should be separated by a space.
 * A blank space should not be used between a function name and its opening parenthesis. This helps to distinguish keywords from function calls.
 * A blank space should appear after commas in argument lists.
 * All binary operators except `.` should be separated from their operands by spaces.
 * Unary operator should never be separated from their operands with spaces.
 * The expressions in a for statement should be separated by spaces.
 * Casts should not be followed by a blank space.

For example:
```
for (i = 0; i < 100; i++) {
    result = (a * a) + (b * b) + (2 * a * b);

    if (result < 100) {
        result = -result;
    }

    a = readInteger();
    b = readInteger();
}

printf("The result is %d.\n", result);
truncate((int64_t*)array, (int16_t)result);
```