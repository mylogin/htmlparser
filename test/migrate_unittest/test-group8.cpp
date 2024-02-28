#include <iostream>
#include <gtest/gtest.h>
#include <string>
#include "html.hpp"

using namespace std;
using namespace html;

//test71
TEST(test, textnodeInBlockIndent){
    string html = "<div>\n{{ msg }} \n </div>\n<div>\n{{ msg }} \n </div>";
    parser parser;
    node_ptr doc = parser.parse(html);
    ASSERT_EQ("<div> {{ msg }} </div>\n<div> {{ msg }} </div>", doc->to_html());
}

//test72
TEST(test, stripTrailing){
    string html = "<p> This <span>is </span>fine. </p>";
    parser parser;
    node_ptr doc = parser.parse(html);
    ASSERT_EQ("<p> This <span>is </span>fine. </p>", doc->to_html());
}

//test73
TEST(test, divAInlineable){
    string html = "<body><div> <a>Text</a>";
    parser parser;
    node_ptr doc = parser.parse(html);
    ASSERT_EQ("<body>\n\t<div><a>Text</a></div>\n</body>", doc->to_html());
}

//test74
TEST(test, noDanglingSpaceAfterCustomElement){
    string html = "<bar><p/>\n</bar>";
    parser parser;
    node_ptr doc = parser.parse(html);
    ASSERT_EQ("<bar>\n\t<p />\n</bar>", doc->to_html());

    html = "<foo>\n  <bar />\n</foo>";
    doc = parser.parse(html);
    ASSERT_EQ("<foo>\n\t<bar />\n</foo>", doc->to_html());
}

//test75
TEST(test, rubyInline){
    string html = "<ruby>T<rp>(</rp><rtc>!</rtc><rt>)</rt></ruby>";
    parser parser;
    node_ptr doc = parser.parse(html);
    ASSERT_EQ("<ruby>T\n\t<rp>(</rp>\n\t<rtc>!</rtc>\n\t<rt>)</rt>\n</ruby>", doc->to_html());
}


GTEST_API_ int main(int argc, char ** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
