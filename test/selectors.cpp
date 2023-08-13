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
	std::vector<html::node> ret;
	static html::parser p;
	static html::node_ptr res;
};

html::parser Selectors::p;
html::node_ptr Selectors::res;

class SelectorsCb: public ::testing::Test {
protected:
	virtual void TearDown() override {
		p.clear_callbacks();
		res.clear();
	}
	void find(std::string sel) {
		p.set_callback(sel, [&](html::node& n) {
			if(n.type_node == html::node_t::tag && n.type_tag == html::tag_t::open) {
				res.push_back(n);
			}
		});
		p.parse(check);
	}
	html::parser p;
	std::vector<html::node> res;
};

TEST_F(Selectors, All) {
	auto sel = res->select("*");
	EXPECT_EQ(sel.size(), 11);
}

TEST_F(Selectors, Nested) {
	auto sel = res->select("body p i");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "i");
}

TEST_F(Selectors, Tag) {
	auto sel = res->select("meta");
	ASSERT_EQ(sel.size(), 2);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "meta");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "meta");
}

TEST_F(Selectors, Id) {
	auto sel = res->select("#div_id");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "div");
}

TEST_F(Selectors, Class) {
	auto sel = res->select(".class_name");
	ASSERT_EQ(sel.size(), 2);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "i");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "b");
}

TEST_F(Selectors, First) {
	auto sel = res->select(":first");
	ASSERT_EQ(sel.size(), 5);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "html");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "head");
	EXPECT_STREQ(sel[2]->tag_name.c_str(), "meta");
	EXPECT_STREQ(sel[3]->tag_name.c_str(), "h1");
	EXPECT_STREQ(sel[4]->tag_name.c_str(), "i");
}

TEST_F(Selectors, Last) {
	auto sel = res->select(":last");
	ASSERT_EQ(sel.size(), 5);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "html");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "title");
	EXPECT_STREQ(sel[2]->tag_name.c_str(), "body");
	EXPECT_STREQ(sel[3]->tag_name.c_str(), "p");
	EXPECT_STREQ(sel[4]->tag_name.c_str(), "b");
}

TEST_F(Selectors, Index) {
	auto sel = res->select(":eq(1)");
	ASSERT_EQ(sel.size(), 4);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "meta");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "body");
	EXPECT_STREQ(sel[2]->tag_name.c_str(), "div");
	EXPECT_STREQ(sel[3]->tag_name.c_str(), "b");
}

TEST_F(Selectors, Greater) {
	auto sel = res->select(":gt(1)");
	ASSERT_EQ(sel.size(), 2);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "title");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "p");
}

TEST_F(Selectors, Less) {
	auto sel = res->select(":lt(1)");
	ASSERT_EQ(sel.size(), 5);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "html");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "head");
	EXPECT_STREQ(sel[2]->tag_name.c_str(), "meta");
	EXPECT_STREQ(sel[3]->tag_name.c_str(), "h1");
	EXPECT_STREQ(sel[4]->tag_name.c_str(), "i");
}

TEST_F(Selectors, AttrExist) {
	auto sel = res->select("[attr]");
	ASSERT_EQ(sel.size(), 2);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "i");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "b");
}

TEST_F(Selectors, AttrEqual) {
	auto sel = res->select("[attr='attr_val2']");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "b");
}

TEST_F(Selectors, AttrNotEqual) {
	auto sel = res->select("[attr!='attr_val2']");
	EXPECT_EQ(sel.size(), 10);
}

TEST_F(Selectors, AttrStartWith) {
	auto sel = res->select("[attr^='attr']");
	ASSERT_EQ(sel.size(), 2);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "i");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "b");
}

TEST_F(Selectors, AttrEndWith) {
	auto sel = res->select("[attr$='val1']");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "i");
}

TEST_F(Selectors, AttrContains) {
	auto sel = res->select("[attr2*='alu']");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "h1");
}

TEST_F(Selectors, AttrAnyOf) {
	auto sel = res->select("#h1_id,p,i");
	ASSERT_EQ(sel.size(), 3);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "h1");
	EXPECT_STREQ(sel[1]->tag_name.c_str(), "p");
	EXPECT_STREQ(sel[2]->tag_name.c_str(), "i");
}

TEST_F(Selectors, AttrAllOf) {
	auto sel = res->select("h1#h1_id.h1_class:first:eq(0):lt(1)[attr2][attr2*='alu']");
	ASSERT_EQ(sel.size(), 1);
	EXPECT_STREQ(sel[0]->tag_name.c_str(), "h1");
}

TEST_F(SelectorsCb, All) {
	find("*");
	EXPECT_EQ(res.size(), 11);
}

TEST_F(SelectorsCb, Tag) {
	find("meta");
	ASSERT_EQ(res.size(), 2);
	EXPECT_STREQ(res[0].tag_name.c_str(), "meta");
	EXPECT_STREQ(res[1].tag_name.c_str(), "meta");
}
TEST_F(SelectorsCb, Id) {
	find("#div_id");
	ASSERT_EQ(res.size(), 1);
	EXPECT_STREQ(res[0].tag_name.c_str(), "div");
}

TEST_F(SelectorsCb, Class) {
	find(".class_name");
	ASSERT_EQ(res.size(), 2);
	EXPECT_STREQ(res[0].tag_name.c_str(), "i");
	EXPECT_STREQ(res[1].tag_name.c_str(), "b");
}

TEST_F(SelectorsCb, First) {
	find(":first");
	ASSERT_EQ(res.size(), 5);
	EXPECT_STREQ(res[0].tag_name.c_str(), "html");
	EXPECT_STREQ(res[1].tag_name.c_str(), "head");
	EXPECT_STREQ(res[2].tag_name.c_str(), "meta");
	EXPECT_STREQ(res[3].tag_name.c_str(), "h1");
	EXPECT_STREQ(res[4].tag_name.c_str(), "i");
}

TEST_F(SelectorsCb, Index) {
	find(":eq(1)");
	ASSERT_EQ(res.size(), 4);
	EXPECT_STREQ(res[0].tag_name.c_str(), "meta");
	EXPECT_STREQ(res[1].tag_name.c_str(), "body");
	EXPECT_STREQ(res[2].tag_name.c_str(), "div");
	EXPECT_STREQ(res[3].tag_name.c_str(), "b");
}

TEST_F(SelectorsCb, Greater) {
	find(":gt(1)");
	ASSERT_EQ(res.size(), 2);
	EXPECT_STREQ(res[0].tag_name.c_str(), "title");
	EXPECT_STREQ(res[1].tag_name.c_str(), "p");
}

TEST_F(SelectorsCb, Less) {
	find(":lt(1)");
	ASSERT_EQ(res.size(), 5);
	EXPECT_STREQ(res[0].tag_name.c_str(), "html");
	EXPECT_STREQ(res[1].tag_name.c_str(), "head");
	EXPECT_STREQ(res[2].tag_name.c_str(), "meta");
	EXPECT_STREQ(res[3].tag_name.c_str(), "h1");
	EXPECT_STREQ(res[4].tag_name.c_str(), "i");
}

TEST_F(SelectorsCb, AttrExist) {
	find("[attr]");
	ASSERT_EQ(res.size(), 2);
	EXPECT_STREQ(res[0].tag_name.c_str(), "i");
	EXPECT_STREQ(res[1].tag_name.c_str(), "b");
}

TEST_F(SelectorsCb, AttrEqual) {
	find("[attr='attr_val2']");
	ASSERT_EQ(res.size(), 1);
	EXPECT_STREQ(res[0].tag_name.c_str(), "b");
}

TEST_F(SelectorsCb, AttrNotEqual) {
	find("[attr!='attr_val2']");
	EXPECT_EQ(res.size(), 10);
}

TEST_F(SelectorsCb, AttrStartWith) {
	find("[attr^='attr']");
	ASSERT_EQ(res.size(), 2);
	EXPECT_STREQ(res[0].tag_name.c_str(), "i");
	EXPECT_STREQ(res[1].tag_name.c_str(), "b");
}

TEST_F(SelectorsCb, AttrEndWith) {
	find("[attr$='val1']");
	ASSERT_EQ(res.size(), 1);
	EXPECT_STREQ(res[0].tag_name.c_str(), "i");
}

TEST_F(SelectorsCb, AttrContains) {
	find("[attr2*='alu']");
	ASSERT_EQ(res.size(), 1);
	EXPECT_STREQ(res[0].tag_name.c_str(), "h1");
}

TEST_F(SelectorsCb, AttrAnyOf) {
	find("#h1_id,p,i");
	ASSERT_EQ(res.size(), 3);
	EXPECT_STREQ(res[0].tag_name.c_str(), "h1");
	EXPECT_STREQ(res[1].tag_name.c_str(), "p");
	EXPECT_STREQ(res[2].tag_name.c_str(), "i");
}

TEST_F(SelectorsCb, AttrAllOf) {
	find("h1#h1_id.h1_class:first:eq(0):lt(1)[attr2][attr2*='alu']");
	ASSERT_EQ(res.size(), 1);
	EXPECT_STREQ(res[0].tag_name.c_str(), "h1");
}