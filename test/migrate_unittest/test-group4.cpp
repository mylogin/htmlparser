#include <iostream>
#include <gtest/gtest.h>
#include <string>
#include "html.hpp"

using namespace std;
using namespace html;

//test31
// issue why ""
TEST(test, findsCharsetInMalformedMeta){
    string h = R"(<meta http-equiv="Content-Type" content="text/html; charset=gb2312" />)";
    parser parser;
    node_ptr doc = parser.parse(h);
    // ASSERT_EQ ("gb2312", doc->select("meta")->get_attr("content"));
}

//test32
TEST(test, testRelaxedTags){
    parser parser;
    node_ptr doc = parser.parse("<abc_def id=1>Hello</abc_def> <abc-def>There</abc-def>");
    ASSERT_EQ ("<abc_def id=\"1\">Hello</abc_def>\n<abc-def>There</abc-def>", doc->to_html());
}

//test33
TEST(test, testSpanContents){
    parser parser;
    node_ptr doc = parser.parse("<span>Hello <div>there</div> <span>now</span></span>");

    ASSERT_EQ ("<span>Hello \n\t<div>there</div>\n\t<span>now</span>\n</span>", doc->to_html());
}

//test34
// not support noscript
TEST(test, testNoImagesInNoScriptInHead){
    parser parser;
    node_ptr doc = parser.parse("<html><head><noscript><img src='foo'></noscript></head><body><p>Hello</p></body></html>");
 
    // <noscript>&lt;img src=\"foo\"&gt;</noscript>
    ASSERT_EQ ("<html>\n\t<head>\n\t\t<noscript><img src='foo'></noscript>\n\t</head>\n\t<body>\n\t\t<p>Hello</p>\n\t</body>\n</html>", doc->to_html());
}

//test35
TEST(test, testAFlowContents){
    parser parser;
    node_ptr doc = parser.parse("<a>Hello <div>there</div> <span>now</span></a>");
 
    ASSERT_EQ ("<a>Hello \n\t<div>there</div>\n\t<span>now</span>\n</a>", doc->to_html());
}

//test36
TEST(test, testFontFlowContents){
    parser parser;
    node_ptr doc = parser.parse("<font>Hello <div>there</div> <span>now</span></font>");
 
    ASSERT_EQ ("<font>Hello \n\t<div>there</div>\n\t<span>now</span>\n</font>", doc->to_html());
}

//test37
TEST(test, reconstructFormattingElements){
    string h = "<p><b class=one>One <i>Two <b>Three</p><p>Hello</p>";
    parser parser;
    node_ptr doc = parser.parse(h);
 
    ASSERT_EQ ("<p><b class=\"one\">One <i>Two <b>Three</b></i></b></p>\n<p>Hello</p>", doc->to_html());
}

//test38
TEST(test, commentBeforeHtml){
    string h = "<!-- comment --><!-- comment 2 --><p>One</p>";
    parser parser;
    node_ptr doc = parser.parse(h);
 
    ASSERT_EQ ("<!-- comment --><!-- comment 2 -->\n<p>One</p>", doc->to_html());
}

//test39
TEST(test, emptyTdTag){
    string h = "<table><tr><td>One</td><td id='2' /></tr></table>";
    parser parser;
    node_ptr doc = parser.parse(h);
    ASSERT_EQ("<tr>\n\t<td>One</td>\n\t<td id=\"2\" />\n</tr>", doc->select("tr")->get_children()[0]->to_html());
}

//test40
TEST(test, handlesUnclosedScriptAtEof){
    parser parser;
    ASSERT_EQ ("Data", parser.parse("<script>Data")->select("script")->get_children()[0]->to_text());
    ASSERT_EQ ("Data", parser.parse("<script>Data<")->select("script")->get_children()[0]->to_text());
    ASSERT_EQ ("Data", parser.parse("<script>Data</sc")->select("script")->get_children()[0]->to_text());
}


GTEST_API_ int main(int argc, char ** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
