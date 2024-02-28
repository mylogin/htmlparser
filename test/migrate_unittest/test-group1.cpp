#include <iostream>
#include <gtest/gtest.h>
#include <string>
#include "html.hpp"

using namespace std;
using namespace html;


//test1
TEST(test, dropsUnterminatedTag){
    string h1 = "<p";
    parser parse;
    node_ptr doc = parse.parse(h1);
    ASSERT_EQ(0, doc->get_attr("p").size());
    ASSERT_EQ("", doc->to_text()); 

    string h2="<div id=1<p id='2'";
    doc=parse.parse(h2);
    ASSERT_EQ("", doc->to_text()); 
}

//test2
TEST(test, testByAttributeRegexCombined){
    parser parse;
    node_ptr doc = parse.parse("<div><table class=x><td>Hello</td></table></div>");
    node_ptr els = doc->select("div table[class~=x|y]");
    
    ASSERT_EQ(1, els->size());
    ASSERT_EQ("Hello", els->to_text());
}

//test3
TEST(test, testAllWithClass){
    string h = "<p class=first></p>One<p class=first>Two<p>Three";
    parser parse;
    node_ptr doc = parse.parse(h);
    node_ptr ps = doc->select("[class*=first]");
    // The last text node, although it does not have a class attribute itself,
    // it is selected because its parent node <p> has a class attribute value that includes "first".
    ASSERT_EQ(3, ps->size());
}

//test4
// select is case sensitive.
TEST(test, caseInsensitive){
    string h = "<div tItle=bAr></div>";
    parser parse;
    node_ptr doc = parse.parse(h);
    // ASSERT_EQ(1, doc->select("DiV")->size());
    // ASSERT_EQ(1, doc->select("dIv")->size());
}

//test5
TEST(test, notAdjacent){
    parser parse;
    string h = "<ol><li id=1>One<li id=2>Two<li id=3>Three</ol>";
    node_ptr doc = parse.parse(h);
    node_ptr sibs = doc->select("li#1 + li#3");
    ASSERT_EQ(0, sibs->size());
}

//test6
TEST(test, selectSameElements){
    parser parse;
    string html = "<div>one</div><div>one</div>";
    node_ptr doc = parse.parse(html);
    node_ptr els = doc->select("div");
    ASSERT_EQ(2, els->size());
    
    // not support :contains
    // node_ptr subSelect = els->select(":contains(one)");
    // ASSERT_EQ(2, subSelect->size());
}

//test7
TEST(test, matchTextAttributes){
    parser parse;
    node_ptr doc = parse.parse("<div><p class=one>One<br>Two<p class=two>Three<br>Four");

    //":matchText" selector is not a standard selector in HTML.
    node_ptr els = doc->select("p.two:matchText:last-child");

    ASSERT_EQ(0, els->size());
    // ASSERT_EQ("Four", els->to_text());
}

//test8
TEST(test, startsWithBeginsWithSpace){
    parser parse;
    node_ptr doc = parse.parse("<small><a href=\" mailto:abc@def.net\">(abc@def.net)</a></small>");

    node_ptr els = doc->select("a[href^=' mailto']");

    ASSERT_EQ(1, els->size());
}

//test9
TEST(test, endsWithEndsWithSpaces){
    parser parse;
    node_ptr doc = parse.parse("<small><a href=\" mailto:abc@def.net \">(abc@def.net)</a></small>");

    node_ptr els = doc->select("a[href$='.net ']");

    ASSERT_EQ(1, els->size());
}

//test10
TEST(test, parsesQuiteRoughAttributes){
    string html = "<p =a>One<a <p>Something</p>Else";
    parser parse;
    node_ptr doc = parse.parse(html);

    // <p =a=\"\">One<a <p=\"\">Something</a></p>\nElse
    ASSERT_EQ("<p =a=\"\">One<a <p=\"\">Something</a></p>\nElse", doc->to_html());

    doc = parse.parse("<p .....>");
    ASSERT_EQ("<p .....=\"\"></p>", doc->to_html());
}

GTEST_API_ int main(int argc, char ** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

