/* 
 * CS:APP Data Lab                                                //name
 * <Please put your name and userid here>                         //yuxin202108040303
 * WARNING: Do not include the <stdio.h> header;                  //note on head file
 */

#if 0
INTEGER CODING RULES:
 
  Replace the "return" statement in each function with C code that implements(achieve) the function. Your code 
  must conform to(be consistent to) the following style:          //work:change return job
           
  int Funct(arg1, arg2, ...) {
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;                                                //brief introduction,definition,equality and return
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.         //(use int from 0 to 255) 
                                                                   //(function arguments and local variables)
                                                                   //! ~
                                                                   //& ^ | + << >>
    

  You are expressly forbidden to:
                                                                   //not if, do, while, for, switch, etc.
                                                                   //not macros
                                                                   //not additional functions
                                                                   //not call any functions.
                                                                   //not any other operations, such as &&, ||, -, or ?:
                                                                   //not any form of casting.
                                                                   //not any data type other than int.  

 
  You may assume that your machine:                                //some more warnings
  2. Performs right shifts arithmetically.

EXAMPLES OF ACCEPTABLE CODING STYLE:                               //two example
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

                                                                   //can use looping and conditional control
                                                                   //can use arbitrary integer and unsigned constants.


                                                                   //not any macros.
                                                                   //not any additional functions in this file.
                                                                   //not call any functions.
                                                                   //not any form of casting.
                                                                   //not any data type other than int or unsigned
                                                                     not any floating point data types, operations, or constants.


NOTES:
                                                                   //use the dlc to check the legality of your solutions.
                                                                   //Each function has a maximum number of operators 
                                                                     (! ~ & ^ | + << >>) 
                                                                     you may use as many of '=' as you want
                                                                   //use the btest test harness to check your functions
                                                                   //use the BDD checker to verify your functions
                                                                   //The maximum number of ops for each function is given in the
                                                                     header comment for each function. 
                                                                   //If there are any inconsistencies 
                                                                     consider this file the authoritative source.

/*        
 * STEP 2: Modify the following functions according the coding rules.
 *                                                                 //improve
 *   IMPORTANT. TO AVOID GRADING SURPRISES:                        

 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  return ~((~x)|(~y));
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  return (x>>(n<<3))&0xff;

}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
  int f=(x>>31)&0x1;
  int l=~(f<<(32+~n)<<1)+1;
  int r=(x>>n)^l;
  return r;
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
  int temp=(((0x1<<8|0x1)<<8|0x1)<<8|0x1)<<8|0x1;
  int val=temp&x;
  val+=temp&(x>>1);
  val+=temp&(x>>2);
  val+=temp&(x>>3);
  val+=temp&(x>>4);
  val+=temp&(x>>5);
  val+=temp&(x>>6);
  val+=temp&(x>>7);                       //8位一组
  val+=(val>>16);
  val+=(val>>8);
  return val&0xff;
}
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
  int t=(~x+1);
  int f=~((x|t)>>31)&0x1;
  return f;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return (1<<31);
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
  int t=x>>(n+(~0));
  int r=(!t|!(t+1));
  return r;
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
    int f=x>>31&0x1;
    int t=x<<(32+(~n))<<1;
    return (x>>n)+(f&(!!t));
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return (~x+1);
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
  return !((x>>31&0x1)|!x);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
   int sx =(x>>31)&0x1;       
   int sy =(y>>31)&0x1;       
   int ifS=!(sx^sy);
   int x1=x+~y+1;
   int x2=((x1>>31&0x1)|!x1);
   return (ifS&x2)|((!ifS)&sx);
}
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2(int x) {
  int c=0;
  c=(!!(x>>16))<<4;
  c=c+((!!(x>>(8+c)))<<3);
  c=c+((!!(x>>(4+c)))<<2);
  c=c+((!!(x>>(2+c)))<<1);
  c=c+(!!(x>>(1+c)));
  return c;
}
/* 
 * float_neg - Return -f for
 *   When NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_neg(unsigned uf) {
     unsigned s=uf<<1;
     if((s&0xff000000)==0xff000000&&(s<<8)!=0)
     return uf;
     return uf^0x80000000;

}
/* 
 * float_i2f - Return (float) x
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */


unsigned float_i2f(int x) {
	unsigned s=x&(1<<31);            
	int i=30; 
	int exp=(x>>31)?158:0;          
	int frac=0;                     
	int delta;                     
	int frac_mask=(1<<23)-1;        
	if(x<<1)                        
	{
	    if(x<0)
	        x=-x;                   
	    while(!((x>>i)&1))           
            i--;
	    exp=i+127;                  
		x=x<<(31-i);                 
		frac=frac_mask&(x>>8);       
		x=x&0xff;                     
		delta=x>128||((x==128)&&(frac&1));
		frac+=delta;
		if(frac>>23)                
		{
		    frac&=frac_mask;        
			exp+=1;                   
	    }
	    
}                             
    return s|(exp<<23)|frac; 
}

/* 
 * float_twice - Return 2*f 
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {  
     unsigned s=uf<<1;
     if((s&0xff000000)==0xff000000||uf==0)
     return uf;
     if((s>>24)==0)
     return ((uf>>31)<<31)|s;
     return uf+0x00800000;
}
