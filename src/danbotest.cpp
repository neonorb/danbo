/*
 * test.cpp
 *
 *  Created on: Oct 21, 2016
 *      Author: chris13524
 */

#ifdef ALLOW_TEST

#include <danbo.h>
#include <feta.h>
#include <danbotest.h>
#include <fetatest.h>

using namespace feta;
using namespace danbo;

namespace danbotest {

#include "danbotest-grammar"

static void danbo() {
#ifdef MEMORY_LOG
	// get allocated count
	uint64 origionalAllocatedCount = getAllocatedCount();
#endif
	
	// parser must fail
	{
		ParseResult<D_ST(letter)> status = D_PARSE(letter, "ab");
		assert(status.status == ParseStatus::FAIL, "parser passed when it was suppose to fail");
		assert(status.failedAt == 1, "incorrect failedAt");
	}
	
	// choose the right character
	{
		ParseResult<D_ST(letter)> status = D_PARSE(letter, "a");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->choice == D_ST(letter)::C_la, "wrong choice");
		delete status.tree;
	}
	{
		ParseResult<D_ST(letter)> status = D_PARSE(letter, "b");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->choice == D_ST(letter)::C_lb, "wrong choice");
		delete status.tree;
	}
	
	// match literal
	{
		ParseResult<D_ST(lit)> status = D_PARSE(lit, "(a)");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->nletter->choice == D_ST(letter)::C_la, "wrong choice");
		delete status.tree;
	}
	{
		ParseResult<D_ST(lit)> status = D_PARSE(lit, "(b)");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->nletter->choice == D_ST(letter)::C_lb, "wrong choice");
		delete status.tree;
	}
	
	// variable
	{
		ParseResult<D_ST(variablelits)> status = D_PARSE(variablelits, "");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->symbols.size() == 0, "wrong lit count");
		delete status.tree;
	}
	{
		ParseResult<D_ST(variablelits)> status = D_PARSE(variablelits, "(a)");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->symbols.size() == 1, "wrong lit count");
		assert(status.tree->symbols.get(0)->nletter->choice == D_ST(letter)::C_la, "wrong choice");
		delete status.tree;
	}
	{
		ParseResult<D_ST(variablelits)> status = D_PARSE(variablelits, "(a)(b)");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->symbols.size() == 2, "wrong lit count");
		assert(status.tree->symbols.get(0)->nletter->choice == D_ST(letter)::C_la, "wrong choice");
		assert(status.tree->symbols.get(1)->nletter->choice == D_ST(letter)::C_lb, "wrong choice");
		delete status.tree;
	}
	
	// many
	{
		ParseResult<D_ST(manylits)> status = D_PARSE(manylits, "");
		assert(status.status == ParseStatus::FAIL, "parser passed when it was suppose to fail");
	}
	{
		ParseResult<D_ST(manylits)> status = D_PARSE(manylits, "(a)");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->symbols.size() == 1, "wrong lit count");
		assert(status.tree->symbols.get(0)->nletter->choice == D_ST(letter)::C_la, "wrong choice");
		delete status.tree;
	}
	{
		ParseResult<D_ST(manylits)> status = D_PARSE(manylits, "(a)(b)");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->symbols.size() == 2, "wrong lit count");
		assert(status.tree->symbols.get(0)->nletter->choice == D_ST(letter)::C_la, "wrong choice");
		assert(status.tree->symbols.get(1)->nletter->choice == D_ST(letter)::C_lb, "wrong choice");
		delete status.tree;
	}
	
	// optional
	{
		ParseResult<D_ST(optionala)> status = D_PARSE(optionala, "");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->exists == false, "tree exists");
		assert(status.tree->tree == NULL, "tree exists");
		delete status.tree;
	}
	{
		ParseResult<D_ST(optionala)> status = D_PARSE(optionala, "a");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->exists == true, "tree exists");
		assert(status.tree->tree != NULL, "tree exists");
		delete status.tree;
	}
	
	// operator precedence
	{
		ParseResult<D_ST(expression)> status = D_PARSE(expression, "v");
		assert(status.status == ParseStatus::OK, "parser failed");
		assertnm(status.tree->choice == D_ST(expression)::C_value);
		delete status.tree;
	}
	{
		ParseResult<D_ST(expression)> status = D_PARSE(expression, "(v)");
		assert(status.status == ParseStatus::OK, "parser failed");
		assertnm(status.tree->choice == D_ST(expression)::C_subexpression);
		assertnm(status.tree->subexpression->expression->choice == D_ST(expression)::C_value);
		delete status.tree;
	}
	{
		ParseResult<D_ST(expression)> status = D_PARSE(expression, "v+v");
		assert(status.status == ParseStatus::OK, "parser failed");
		assertnm(status.tree->choice == D_ST(expression)::C_addition);
		assertnm(status.tree->addition->rest->symbols.size() == 1);
		assertnm(status.tree->addition->rest->symbols.get(0)->operand->choice == D_ST(additionOptions)::C_value);
		delete status.tree;
	}
	{
		ParseResult<D_ST(expression)> status = D_PARSE(expression, "v*v");
		assert(status.status == ParseStatus::OK, "parser failed");
		assertnm(status.tree->choice == D_ST(expression)::C_multiplication);
		assertnm(status.tree->multiplication->rest->symbols.size() == 1);
		assertnm(status.tree->multiplication->rest->symbols.get(0)->operand->choice == D_ST(multiplicationOptions)::C_value);
		delete status.tree;
	}
	{
		ParseResult<D_ST(expression)> status = D_PARSE(expression, "v*v+v");
		assert(status.status == ParseStatus::OK, "parser failed");
		assertnm(status.tree->choice == D_ST(expression)::C_addition);
		assertnm(status.tree->addition->first->choice == D_ST(additionOptions)::C_multiplication);
		assertnm(status.tree->addition->first->multiplication->first->choice == D_ST(multiplicationOptions)::C_value); // first value
		assertnm(status.tree->addition->first->multiplication->rest->symbols.size() == 1);
		assertnm(status.tree->addition->first->multiplication->rest->symbols.get(0)->operand->choice == D_ST(multiplicationOptions)::C_value); // second value
		assertnm(status.tree->addition->rest->symbols.size() == 1);
		assertnm(status.tree->addition->rest->symbols.get(0)->operand->choice == D_ST(additionOptions)::C_value); // third value
		delete status.tree;
	}
	{
		ParseResult<D_ST(expression)> status = D_PARSE(expression, "v+v*v");
		assert(status.status == ParseStatus::OK, "parser failed");
		assertnm(status.tree->choice == D_ST(expression)::C_addition);
		assertnm(status.tree->addition->first->choice == D_ST(additionOptions)::C_value); // first value
		assertnm(status.tree->addition->rest->symbols.size() == 1);
		assertnm(status.tree->addition->rest->symbols.get(0)->operand->choice == D_ST(additionOptions)::C_multiplication);
		assertnm(status.tree->addition->rest->symbols.get(0)->operand->multiplication->first->choice == D_ST(multiplicationOptions)::C_value); // second value
		assertnm(status.tree->addition->rest->symbols.get(0)->operand->multiplication->rest->symbols.size() == 1);
		assertnm(status.tree->addition->rest->symbols.get(0)->operand->multiplication->rest->symbols.get(0)->operand->choice == D_ST(multiplicationOptions)::C_value); // third value
		delete status.tree;
	}
	{
		ParseResult<D_ST(expression)> status = D_PARSE(expression, "(v+v)*v");
		assert(status.status == ParseStatus::OK, "parser failed");
		assertnm(status.tree->choice == D_ST(expression)::C_multiplication);
		assertnm(status.tree->multiplication->first->choice == D_ST(multiplicationOptions)::C_subexpression);
		assertnm(status.tree->multiplication->first->subexpression->expression->choice == D_ST(expression)::C_addition);
		assertnm(status.tree->multiplication->first->subexpression->expression->addition->first->choice == D_ST(additionOptions)::C_value); // first value
		assertnm(status.tree->multiplication->first->subexpression->expression->addition->rest->symbols.size() == 1);
		assertnm(status.tree->multiplication->first->subexpression->expression->addition->rest->symbols.get(0)->operand->choice == D_ST(additionOptions)::C_value); // second value
		assertnm(status.tree->multiplication->rest->symbols.size() == 1);
		assertnm(status.tree->multiplication->rest->symbols.get(0)->operand->choice == D_ST(multiplicationOptions)::C_value); // third value
		delete status.tree;
	}

#ifdef MEMORY_LOG
	// get allocated count
	uint64 laterAllocatedCount = getAllocatedCount();

	// confirm no memory leaks
	if(origionalAllocatedCount != laterAllocatedCount) {
		//dumpAllocated();
	}
	assert(origionalAllocatedCount == laterAllocatedCount, "there is a memory leak");
#endif
}

void test() {
	log(" - danbo");
	danbo();
}

}

#endif
