# Style Guidelines

## Indentation

 * Four spaces should be used as the unit of indentation.
 * Do not use tabs for indentation.
 * Avoid lines longer than 80 characters.

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

## Statements

### Simple Statements

 * Each simple statement should occupy a single line.
 * If the statement exceeds 80 characters, wrap it to the next line.

### Compound Statements

 * The body of a compound statement should be indented one more level than the compound statement.
 * The opening brace should be in the same line where the compound statement begins.
 * The closing brace should be in its own line, indented one level lesser than the body.
 * Braces should be used, even when the body consists just one statement.

### If Statement

The form of the if statement should be:
```
if (condition) {
    ...
}

if (condition) {
    ...
}
else {
    ...
}

if (condition) {
    ...
}
else if (condition) {
    ...
}
else if (condition) {
    ...
}
```

### For Statement

The form of the for statement should be:
```
for (initialization; condition; update) {
    ...
}
```

## While Statement

The form of the while statement should be:
```
while (condition) {
    ...
}
```

An empty while statement should be written in the form:
```
while (condition) {
}
```

## Do While Statement

The form of the do-while statement should be:
```
do {
    ...
}
while (condition);
```

## Switch Statement

The form of the switch statement should be:
```
switch (condition) {
    case CONSTANT: {
        ...
        break;
    }

    case CONSTANT: {
        ...
        break;
    }

    case CONSTANT: {
        ...
        break;
    }

    default: {
        ...
        break;
    }
}
```

Obviously, do not include the break statement if the case falls through.