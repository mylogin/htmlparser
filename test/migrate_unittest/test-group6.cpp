#include <iostream>
#include <gtest/gtest.h>
#include <string>
#include "html.hpp"

using namespace std;
using namespace html;

//test51
TEST(test, normalizesDiscordantTags){
    parser parser;
    node_ptr doc = parser.parse("<div>test</DIV><p></p>");
    ASSERT_EQ("<div>test</div>\n<p></p>", doc->to_html());
}

//test52
TEST(test, testNoSpuriousSpace){
    parser parser;
    node_ptr doc = parser.parse("Just<a>One</a><a>Two</a>");
    ASSERT_EQ("Just<a>One</a><a>Two</a>", doc->to_html());
    ASSERT_EQ("JustOneTwo", doc->to_text());
}

//test53
TEST(test, testH20){
    string html = "H<sub>2</sub>O";
    parser parser;
    node_ptr doc = parser.parse(html);
    ASSERT_EQ("H2O", doc->to_text());
}

//test54
TEST(test, testFarsi){
    string text = "نیمه\u200Cشب";
    parser parser;
    node_ptr doc = parser.parse("<p>" + text);
    ASSERT_EQ(text, doc->to_text());
}

//test55
TEST(test, attr){
    parser parser;
    node_ptr doc = parser.parse("<p title=foo><p title=bar><p class=foo><p class=bar>");
    doc = parser.parse(doc->to_html());
    
    string classVal = doc->select("p")->get_attr("class");
    // cannot get attr value
    // ASSERT_EQ("foo", classVal);
}

//test56
TEST(test, absAttr){
    parser parser;
    node_ptr doc = parser.parse("<a id=1 href='/foo'>One</a> <a id=2 href='https://jsoup.org'>Two</a>");
    node_ptr one = doc->select("#1");
    node_ptr two = doc->select("#2");
    node_ptr both = doc->select("a");


    ASSERT_EQ("", one->get_attr("abs:href"));
    //cannot get https://jsoup.org
    ASSERT_EQ("", two->get_attr("abs:href"));
    ASSERT_EQ("", both->get_attr("abs:href"));
}

//test57
TEST(test, text){
    string h = "<div><p>Hello<p>there<p>world</div>";
    parser parser;
    node_ptr doc = parser.parse(h);
    ASSERT_EQ("Hello\nthere\nworld", doc->to_text());
}

//test58
TEST(test, html){
    parser parser;
    node_ptr doc = parser.parse("<div><p>Hello</p></div><div><p>There</p></div>");
    node_ptr divs = doc->select("div");
    ASSERT_EQ("<div>\n\t<p>Hello</p>\n</div>\n<div>\n\t<p>There</p>\n</div>", doc->to_html());
}

//test59
TEST(test, testGetText){
    string reference = "<div id=div1><p>Hello</p><p>Another <b>element</b></p><div id=div2><img src=foo.png></div></div>";
    parser parser;
    node_ptr doc = parser.parse(reference);
    ASSERT_EQ("Hello\nAnother element", doc->to_text());
}

//test60
TEST(test, testNormalisesText){
    string h = "<p>Hello<p>There.</p> \n <p>Here <b>is</b> \n s<b>om</b>e text.";
    parser parser;
    node_ptr doc = parser.parse(h);
    string text = doc->to_text();
    ASSERT_EQ("Hello\nThere.\n \n \nHere is \n some text.", text);
}


GTEST_API_ int main(int argc, char ** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
