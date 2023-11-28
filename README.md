# TPCC - TPC Compiler

TPC is a tiny subset of the C programming language.

TPCC compiles TPC files to assembly ([NASM](https://www.nasm.us/)).

## Make targets

- `make` builds the executable to `bin/tpcc`.
- `make clean` deletes generated files.
- `make test` runs a set of tests to ensure that TPCC is working correctly. It also generates a recap in `test/results.log`
- `make tar` creates an archive containing all the source code. This archive is located to `../tpcc-source-code.tar.gz`.

## Usage

```shell
bin/tpcc [OPTIONS] < yourfile.tpc
```

This compiles `yourfile.tpc` to `_anonymous.asm`.

## Options

| Short version | Long version | Description
| ------------- | ------------ | -----------
| `-h`          | `--help`     | Print help and quit
| `-t`          | `--tree`     | Print syntax tree
| `-s`          | `--symtabs`  | Print all symbol tables

## Return value

- 0 if the source program contains no errors (even if there are warnings)
- 1 if it contains a lexical or syntax error
- 2 if it contains a semantic error
- 3 or more for other types of error

## About the TPC language

Here's an informal description of TPC :

A TPC program is a sequence of functions. Each function consists of local variables declarations and a sequence of instructions. There may be global variables. In this case, they are declared before the functions.

The data types are `int` (signed integer) and `char`. There's no array, enum, struct or union in TPC.

There are four built-in functions for I/O : `putint`, `putchar`, `getint`, `getchar`. Those are written in assembly.

There's no for loop but there are while loops.

Here's an example of a TPC program that computes the gcd of two integers :

```C
int main(void) {
    int x = getint();
    int y = getint();
    putint(gcd(x, y));
    putchar('\n');
    return 0;
}

/* Returns the gcd of two nonnegative integers */
int gcd(int a, int b) {
    if (a < b)
        return gcd(b, a); // ensure a >= b
    if (b == 0)
        return a;
    return gcd(a - b, b);
}
```