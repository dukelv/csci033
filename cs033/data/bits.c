/* 
 * Data Project
 * 
 * <Please put your name and login here>
 * 
 * bits.c - Source file with your solutions to the Project.
 *          This is the file you will hand in.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dpc
 * compiler. You can still use printf() for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Project by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
	Complete the body of each function with one
	or more lines of C code that implements the function. Your code 
	must consist of a sequence of variable declarations, followed by
	a sequence of assignment statements, followed by a return statement.
  
	The general style is as follows:

	int funct(...) {
		/* brief description of how your implementation works */
		int var1 = expr1;
		...
		int varM = exprM;

		varJ = exprJ;
		...
		varN = exprN;

		return exprR;
	}

	Each "expr" is an expression using ONLY the following:
	1. Integer constants 0 through 255 (0xFF), inclusive. You are
	   not allowed to use big constants such as 0xFFFFFFFF.
	2. Function arguments and local variables (no global variables).
	3. Unary integer operations ! ~
	4. Binary integer operations & ^ | + << >>
    
	Some of the problems restrict the set of allowed operators even further.
	Each "expr" may consist of multiple operators. You are not restricted to
	one operator per line.

	You are expressly forbidden to:
	1. Use any control constructs such as if, do, while, for, switch, etc.
	2. Define or use any macros.
	3. Define any additional functions in this file.
	4. Call any functions.
	5. Use any other operations, such as &&, ||, -, or ?:
	6. Use any form of casting.
	7. Use any data type other than int.  This implies that you
	   cannot use arrays, structs, or unions.

 
	You may assume that your machine:
	1. Uses 2s complement, 32-bit representations of integers.
	2. Performs right shifts arithmetically.
	3. Has unpredictable behavior when shifting an integer by more
	   than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
	/*
	 * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
	 */
	int pow2plus1(int x) {
		/* exploit ability of shifts to compute powers of 2 */
		return (1 << x) + 1;
	}

	/*
	 * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
	 */
	int pow2plus4(int x) {
		/* exploit ability of shifts to compute powers of 2 */
		int result = (1 << x);
		result += 4;
		return result;
	}

FLOATING POINT CODING RULES

	For the problem that requires you to implement a floating-point operation,
	the coding rules are less strict.  You are allowed to use looping and
	conditional control.  You are allowed to use both ints and unsigneds.
	You can use arbitrary integer and unsigned constants.

	You are expressly forbidden to:
	1. Define or use any macros.
	2. Define any additional functions in this file.
	3. Call any functions.
	4. Use any form of casting.
	5. Use any data type other than int or unsigned.  This means that you
	   cannot use arrays, structs, or unions.
	6. Use any floating point data types, operations, or constants.


NOTES:
	1. Use the dpc (data project checker) compiler (described in the handout) to 
	   check the legality of your solutions.
	2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
	   that you are allowed to use for your implementation of the function. 
	   The max operator count is checked by dpc. Note that '=' is not 
	   counted; you may use as many of these as you want without penalty.
	3. Use the btest test harness to check your functions for correctness.
	4. Use the btest checker to formally verify your functions.
	5. The maximum number of ops for each function is given in the
	   header comment for each function. If there are any inconsistencies 
	   between the maximum ops in the writeup and in this file, consider
	   this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dpc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the btest checker to formally verify that your solutions produce 
 *      the correct answers.
 */

#endif

/* 
 * bitAnd - Compute x&y using only ~ and | 
 *   Examples: bitAnd(6, 5) = 4
 *             bitAnd(3, 12) = 0
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
    return ~((~x)|(~y));
}
/* 
 * conditional - Compute the result of x ? y : z 
 *   Examples: conditional(2, 4, 5) = 4
 *             conditional(0, 1, 2) = 2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
    return (y&(~(~(!x)+1)))|(z&(~(!x)+1));
}
/* 
 * negate - Return -x
 *   Examples: negate(1) = -1
 *             negate(0) = 0
 *             negate(-33) = 33
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
    return (~x)+1;
}
/* 
 * isEqual - Return 1 if x == y, else return 0
 *   Examples: isEqual(5, 5) = 1
 *             isEqual(4, 5) = 0
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int isEqual(int x, int y) {
 return !(x^y);
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *   Round toward zero
 *   Examples: divpwr2(15, 1) = 7
 *             divpwr2(-33, 4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
    int y = x>>31;
    x = x + ((((~y)+1)&1)<<n) + y;
    return (x >> n);
}
/* 
 * addOK - Return 0 if x+y will overflow, 1 otherwise
 *   Examples: addOK(0x80000000, 0x80000000) = 0
 *             addOK(0x80000000, 0x70000000) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int addOK(int x, int y) {
    int a, b, c;
    a = x>>31;
    b = y>>31;
    c = (x+y)>>31;
    return !((~(a^b))&(a^(c)));
}
/* 
 * absVal - Return the absolute value of x
 *   Examples: absVal(-1) = 1
 *             absVal(33) = 33
 *   You may assume -TMax <= x <= TMax
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 4
 */
int absVal(int x) {
    int y = x>>31;
    return (x^y)+((~y)+1);
}
/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *
 *   Argument is passed as unsigned int, but it is to be
 *   interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *
 *   Fractional portion of value should be truncated.
 *   Anything out of range (including NaN and infinity)
 *   should return 0x80000000u.
 *
 *   Legal ops: Any integer/unsigned operations incl. ||, &&, <, >, etc.
 *   Also allowed: if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
    int emask, fmask, smask, exp, frac, e, m, s, ans;
    emask = 0x7F800000;
    fmask = 0x007FFFFF;
    smask = 0x80000000;
    exp = uf & emask;
    frac = uf & fmask;
    s = uf & smask;
    if((exp==emask)||(exp==0))
    {
	if(exp==0)
	{
	    ans = 0;
	}
	else
	{
	    ans = 0x80000000u;
	}
    }
    else
    {
	e = (exp>>23)-127;
	if((23-e)>=0)
	{
	    frac = frac|0x00800000;
	    m = frac>>(23-e);
	    if((23-e)>23)
	    {
		m = 0;
	    }
	    if(e==0)
	    {
		e=1;
	    }
	    else
	    {
		e=2<<e;
	    }
	    ans = m*e;
	    if(s==smask)
	    {
		ans = -ans;
	    }
	}
	else
	{
	    ans = 0x80000000;
	}
    }// normal
    return ans;
}
