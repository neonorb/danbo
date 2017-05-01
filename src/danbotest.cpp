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
		ParseResult<ST(letter)> status = PARSE(letter, "ab");
		assert(status.status == ParseStatus::FAIL, "parser passed when it was suppose to fail");
		assert(status.failedAt == 1, "incorrect failedAt");
	}
	
	// choose the right character
	{
		ParseResult<ST(letter)> status = PARSE(letter, "a");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->choice == ST(letter)::C_la, "wrong choice");
		delete status.tree;
	}
	{
		ParseResult<ST(letter)> status = PARSE(letter, "b");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->choice == ST(letter)::C_lb, "wrong choice");
		delete status.tree;
	}
	
	// match literal
	{
		ParseResult<ST(expression)> status = PARSE(expression, "(a)");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->nletter->choice == ST(letter)::C_la, "wrong choice");
		delete status.tree;
	}
	{
		ParseResult<ST(expression)> status = PARSE(expression, "(b)");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->nletter->choice == ST(letter)::C_lb, "wrong choice");
		delete status.tree;
	}
	
	// variable
	{
		ParseResult<ST(variableexpressions)> status = PARSE(variableexpressions, "");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->symbols.size() == 0, "wrong expression count");
		delete status.tree;
	}
	{
		ParseResult<ST(variableexpressions)> status = PARSE(variableexpressions, "(a)");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->symbols.size() == 1, "wrong expression count");
		assert(status.tree->symbols.get(0)->nletter->choice == ST(letter)::C_la, "wrong choice");
		delete status.tree;
	}
	{
		ParseResult<ST(variableexpressions)> status = PARSE(variableexpressions, "(a)(b)");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->symbols.size() == 2, "wrong expression count");
		assert(status.tree->symbols.get(0)->nletter->choice == ST(letter)::C_la, "wrong choice");
		assert(status.tree->symbols.get(1)->nletter->choice == ST(letter)::C_lb, "wrong choice");
		delete status.tree;
	}
	
	// many
	{
		ParseResult<ST(manyexpressions)> status = PARSE(manyexpressions, "");
		assert(status.status == ParseStatus::FAIL, "parser passed when it was suppose to fail");
	}
	{
		ParseResult<ST(manyexpressions)> status = PARSE(manyexpressions, "(a)");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->symbols.size() == 1, "wrong expression count");
		assert(status.tree->symbols.get(0)->nletter->choice == ST(letter)::C_la, "wrong choice");
		delete status.tree;
	}
	{
		ParseResult<ST(manyexpressions)> status = PARSE(manyexpressions, "(a)(b)");
		assert(status.status == ParseStatus::OK, "parser failed");
		assert(status.tree->symbols.size() == 2, "wrong expression count");
		assert(status.tree->symbols.get(0)->nletter->choice == ST(letter)::C_la, "wrong choice");
		assert(status.tree->symbols.get(1)->nletter->choice == ST(letter)::C_lb, "wrong choice");
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
