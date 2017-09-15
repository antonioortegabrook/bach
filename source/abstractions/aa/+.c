/**
	@file
	+.c
	
	@name 
	bach.+
	
	@realname 
	bach.+

	@type
	abstraction
	
	@module
	bach

	@author
	bachproject
	
	@digest 
	Quick llll addition
	
	@description
	Performs additions between lllls or
	between lllls and numbers.
	
	@discussion
	The abstraction is equivalent to a sum performed
	in a <o>bach.expr</o> object, with the <m>scalarmode</m>
	attribute set to 1.

	@category
	bach, bach abstractions, bach math

	@keywords
	addition, calculate, compute, sum, number

	@seealso
	bach.expr, bach.-, bach.*, bach./, +, bach.mod+
	
	@owner
	Andrea Agostini
*/

// ---------------
// METHODS
// ---------------

// @method llll @digest Set operand, output result
// @description An llll in the first inlet will set the left
// llll for the addition, and will output the result; an llll
// in the second inlet will set the right llll for the addition,
// without outputting any result. <br />
// @copy BACH_DOC_OPERATION_ABSTRACTIONS_LLLL_AND_NUMBERS_REMARK

// @method bang @digest Output result
// @description Performs and outputs the calculation on the most recently received input lllls.



// ---------------
// ATTRIBUTES
// ---------------

void main_foo() {

llllobj_class_add_out_attr(c, LLLL_OBJ_VANILLA);

}

// ---------------
// INLETS
// ---------------

// @in 0 @type llll @digest Left operand llll
// @in 1 @type llll @digest Right operand llll



// ---------------
// OUTLETS
// ---------------

// @out 0 @type llll @digest The addition of the two operands



// ---------------
// ARGUMENTS
// ---------------

// @arg 0 @name right_operand @optional 1 @type llll @digest Right operand llll
// @description The optional argument is the right operand llll.