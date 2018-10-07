/**
	@file
	sliceheader.c
	
	@name 
	bach.sliceheader
	
	@realname 
	bach.sliceheader

	@type
	abstraction
	
	@module
	bach

	@author
	bachproject
	
	@digest 
	Separate header and body of a gathered syntax
	
	@description
	Takes the gathered syntax of a notation object and separates the
	header part (containing meta-information about the score) and the body part (containing
	the actual content).
	
	@discussion
	@copy BACH_DOC_GENERIC_GATHERED_SYNTAX

	@category
	bach, bach abstractions, bach notation

	@keywords
	header, information, slice, separate, body, gathered syntax, content, split, retrieve
	
	@seealso
	bach.roll, bach.score, bach.slice
	
	@owner
	Daniele Ghisi
*/

// ---------------
// METHODS
// ---------------

// @method llll @digest Separate header and body of gathered syntax
// @description An llll in the first inlet is considered as the 
// gathered syntax of a notation object: its header and body parts are separated, and output
// respectively from the first and second outlets (in right-to-left order, as usual).
// The header part also incorporates the notation object routing symbol (such as <b>roll</b> or
// <b>score</b>), which is a slight abuse, since strictly speaking it is not part of the header.
// <br /> <br />
// @copy BACH_DOC_GENERIC_GATHERED_SYNTAX


// @method bang @digest Separate header and body
// @description Separates header and body of the most recently received input llll.


// ---------------
// ATTRIBUTES
// ---------------
 
void main_foo() {

llllobj_class_add_default_bach_attrs(c, LLLL_OBJ_VANILLA);

}

// ---------------
// INLETS
// ---------------

// @in 0 @type llll @digest The llll containing the gathered syntax of the notation object

// ---------------
// OUTLETS
// ---------------

// @out 0 @type llll @digest The llll containing the header of the notation object (and routing symbol, if any)
// @out 1 @type llll @digest The llll containing the body of the notation object


// ---------------
// ARGUMENTS
// ---------------

// (none)
