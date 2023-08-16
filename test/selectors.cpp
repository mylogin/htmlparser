#include <gtest/gtest.h>
#include "html.hpp"

const char* check = R"html(
	<!DOCTYPE html>
	<html>
		<head>
			<meta charset="utf-8" />
			<meta name="author" content="mylogin" />
			<title>Selectors</title>
		</head>
		<body>
			<h1 id="h1_id" attr2="value" class="h1_class">h1</h1>
			<div id="div_id"></div>
			<p>
				<i attr="attr_val1" class="class_name">italic</i>
				<b attr="attr_val2" class="class_name">bold</b>
			</p>
			<!--comment-->
		</body>
	</html>
)html";

class Selectors: public ::testing::Test {
protected:
	static void SetUpTestSuite() {
		res = p.parse(check);
	}
	void find(std::string s) {
		sel = res->select(s);	
	}
	static html::parser p;
	static html::node_ptr res;
	std::vector<html::node*> sel;
};

html::parser Selectors::p;
html::node_ptr Selectors::res;

class SelectorsCb: public ::testing::Test {
protected:
	void find(std::string s) {
		p.set_callback(s, [&](html::node& n) {
			if(n.type_node == html::node_t::tag && n.type_tag == html::tag_t::open) {
				sel.push_back(&n);
			}
		});
		res = p.parse(check);
	}
	html::parser p;
	html::node_ptr res;
	std::vector<html::node*> sel;
};

TEST_F(Selectors, All) {
	find("*");
	EXPECT_EQ(sel.size(), 11);
}

TEST_F(Selectors, Nested) {
	find("body p i");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "i");
}

TEST_F(Selectors, Tag) {
	find("meta");
	ASSERT_EQ(sel.size(), 2);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "meta");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "meta");
}

TEST_F(Selectors, Id) {
	find("#div_id");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "div");
}

TEST_F(Selectors, Class) {
	find(".class_name");
	ASSERT_EQ(sel.size(), 2);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "i");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "b");
}

TEST_F(Selectors, First) {
	find(":first");
	ASSERT_EQ(sel.size(), 5);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "html");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "head");
	EXPECT_STREQ(sel[2]->tag_name.c_str(), "meta");
	EXPECT_STREQ(sel[3]->tag_name.c_str(), "h1");
	EXPECT_STREQ(sel[4]->tag_name.c_str(), "i");
}

TEST_F(Selectors, Last) {
	find(":last");
	ASSERT_EQ(sel.size(), 5);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "html");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "title");
	EXPECT_STREQ(sel[2]->tag_name.c_str(), "body");
	EXPECT_STREQ(sel[3]->tag_name.c_str(), "p");
	EXPECT_STREQ(sel[4]->tag_name.c_str(), "b");
}

TEST_F(Selectors, Index) {
	find(":eq(1)");
	ASSERT_EQ(sel.size(), 4);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "meta");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "body");
	EXPECT_STREQ(sel[2]->tag_name.c_str(), "div");
	EXPECT_STREQ(sel[3]->tag_name.c_str(), "b");
}

TEST_F(Selectors, Greater) {
	find(":gt(1)");
	ASSERT_EQ(sel.size(), 2);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "title");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "p");
}

TEST_F(Selectors, Less) {
	find(":lt(1)");
	ASSERT_EQ(sel.size(), 5);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "html");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "head");
	EXPECT_STREQ(sel[2]->tag_name.c_str(), "meta");
	EXPECT_STREQ(sel[3]->tag_name.c_str(), "h1");
	EXPECT_STREQ(sel[4]->tag_name.c_str(), "i");
}

TEST_F(Selectors, AttrExist) {
	find("[attr]");
	ASSERT_EQ(sel.size(), 2);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "i");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "b");
}

TEST_F(Selectors, AttrEqual) {
	find("[attr='attr_val2']");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "b");
}

TEST_F(Selectors, AttrNotEqual) {
	find("[attr!='attr_val2']");
	EXPECT_EQ(sel.size(), 10);
}

TEST_F(Selectors, AttrStartWith) {
	find("[attr^='attr']");
	ASSERT_EQ(sel.size(), 2);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "i");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "b");
}

TEST_F(Selectors, AttrEndWith) {
	find("[attr$='val1']");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "i");
}

TEST_F(Selectors, AttrContains) {
	find("[attr2*='alu']");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "h1");
}

TEST_F(Selectors, AnyOf) {
	find("#h1_id,p,i");
	ASSERT_EQ(sel.size(), 3);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "h1");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "p");
	EXPECT_STREQ(sel[2]->tag_name.c_str(), "i");
}

TEST_F(Selectors, AllOf) {
	find("h1#h1_id.h1_class:first:eq(0):lt(1)[attr2][attr2*='alu']");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "h1");
}

TEST_F(Selectors, DirectChild) {
	html::parser lp;
	auto ptr = lp.parse("<div><div><div><p><p><p><b></b><i></i></p></p></p></div></div></div>");
	auto lsel = ptr->select("div>div>div>p>p>p>b,i");
	ASSERT_EQ(lsel.size(), 2);
	EXPECT_STREQ(lsel[0]->tag_name.c_str(), "b");
	EXPECT_STREQ(lsel[1]->tag_name.c_str(), "i");
	lsel = ptr->select("div>p>p>p>b,i");
	ASSERT_EQ(lsel.size(), 2);
	EXPECT_STREQ(lsel[0]->tag_name.c_str(), "b");
	EXPECT_STREQ(lsel[1]->tag_name.c_str(), "i");
	lsel = ptr->select("div>p>p>p>b,i");
	ASSERT_EQ(lsel.size(), 2);
	EXPECT_STREQ(lsel[0]->tag_name.c_str(), "b");
	EXPECT_STREQ(lsel[1]->tag_name.c_str(), "i");
	lsel = ptr->select("p>b,i");
	ASSERT_EQ(lsel.size(), 2);
	EXPECT_STREQ(lsel[0]->tag_name.c_str(), "b");
	EXPECT_STREQ(lsel[1]->tag_name.c_str(), "i");

	lsel = ptr->select("div>p>p>b");
	ASSERT_EQ(lsel.size(), 0);
}

TEST_F(Selectors, SelCombination) {
	html::parser lp;
	auto ptr = lp.parse("<div><div><div><p><p><p><b></b><i></i></p></p></p></div></div></div>");
	auto lsel = ptr->select("div>div>div p>p>p b");
	ASSERT_EQ(lsel.size(), 1);
	EXPECT_STREQ(lsel[0]->tag_name.c_str(), "b");
	lsel = ptr->select("div>div div p>p p b");
	ASSERT_EQ(lsel.size(), 1);
	EXPECT_STREQ(lsel[0]->tag_name.c_str(), "b");
	lsel = ptr->select("div div>div p>p>b,i");
	ASSERT_EQ(lsel.size(), 2);
	EXPECT_STREQ(lsel[0]->tag_name.c_str(), "b");
	EXPECT_STREQ(lsel[1]->tag_name.c_str(), "i");
	lsel = ptr->select("div p p>b");
	ASSERT_EQ(lsel.size(), 1);
	EXPECT_STREQ(lsel[0]->tag_name.c_str(), "b");
	lsel = ptr->select("div div div p>p>p b");
	ASSERT_EQ(lsel.size(), 1);
	EXPECT_STREQ(lsel[0]->tag_name.c_str(), "b");
	lsel = ptr->select("div>div>p b");
	ASSERT_EQ(lsel.size(), 1);
	EXPECT_STREQ(lsel[0]->tag_name.c_str(), "b");

	lsel = ptr->select("div div>p>p>b");
	ASSERT_EQ(lsel.size(), 0);
}

TEST_F(SelectorsCb, All) {
	find("*");
	EXPECT_EQ(sel.size(), 11);
}

TEST_F(SelectorsCb, Tag) {
	find("meta");
	ASSERT_EQ(sel.size(), 2);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "meta");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "meta");
}
TEST_F(SelectorsCb, Id) {
	find("#div_id");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "div");
}

TEST_F(SelectorsCb, Class) {
	find(".class_name");
	ASSERT_EQ(sel.size(), 2);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "i");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "b");
}

TEST_F(SelectorsCb, First) {
	find(":first");
	ASSERT_EQ(sel.size(), 5);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "html");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "head");
	EXPECT_STREQ(sel[2]->tag_name.c_str(), "meta");
	EXPECT_STREQ(sel[3]->tag_name.c_str(), "h1");
	EXPECT_STREQ(sel[4]->tag_name.c_str(), "i");
}

TEST_F(SelectorsCb, Index) {
	find(":eq(1)");
	ASSERT_EQ(sel.size(), 4);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "meta");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "body");
	EXPECT_STREQ(sel[2]->tag_name.c_str(), "div");
	EXPECT_STREQ(sel[3]->tag_name.c_str(), "b");
}

TEST_F(SelectorsCb, Greater) {
	find(":gt(1)");
	ASSERT_EQ(sel.size(), 2);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "title");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "p");
}

TEST_F(SelectorsCb, Less) {
	find(":lt(1)");
	ASSERT_EQ(sel.size(), 5);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "html");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "head");
	EXPECT_STREQ(sel[2]->tag_name.c_str(), "meta");
	EXPECT_STREQ(sel[3]->tag_name.c_str(), "h1");
	EXPECT_STREQ(sel[4]->tag_name.c_str(), "i");
}

TEST_F(SelectorsCb, AttrExist) {
	find("[attr]");
	ASSERT_EQ(sel.size(), 2);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "i");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "b");
}

TEST_F(SelectorsCb, AttrEqual) {
	find("[attr='attr_val2']");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "b");
}

TEST_F(SelectorsCb, AttrNotEqual) {
	find("[attr!='attr_val2']");
	EXPECT_EQ(sel.size(), 10);
}

TEST_F(SelectorsCb, AttrStartWith) {
	find("[attr^='attr']");
	ASSERT_EQ(sel.size(), 2);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "i");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "b");
}

TEST_F(SelectorsCb, AttrEndWith) {
	find("[attr$='val1']");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "i");
}

TEST_F(SelectorsCb, AttrContains) {
	find("[attr2*='alu']");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "h1");
}

TEST_F(SelectorsCb, AnyOf) {
	find("#h1_id,p,i");
	ASSERT_EQ(sel.size(), 3);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "h1");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "p");
	EXPECT_STREQ(sel[2]->tag_name.c_str(), "i");
}

TEST_F(SelectorsCb, AllOf) {
	find("h1#h1_id.h1_class:first:eq(0):lt(1)[attr2][attr2*='alu']");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "h1");
}