#include <iostream>
#include <gtest/gtest.h>
#include <string>
#include "html.hpp"

using namespace std;
using namespace html;

//test41
TEST(test, noImplicitFormForTextAreas){
    parser parser;
    node_ptr doc = parser.parse("<textarea>One</textarea>");
    ASSERT_EQ ("<textarea>One</textarea>", doc->to_html());
}

//test42
TEST(test, handles0CharacterAsText){
    parser parser;
    node_ptr doc = parser.parse("0<p>0</p>");
    ASSERT_EQ ("0\n<p>0</p>", doc->to_html());
}

//test43
TEST(test, handlesNullInData){
    parser parser;
    node_ptr doc = parser.parse("<p id=\u0000>Blah \u0000</p>");
    //
    ASSERT_EQ ("", doc->to_html());
}

//test44
TEST(test, handlesNewlinesAndWhitespaceInTag){
    parser parser;
    node_ptr doc = parser.parse("<a \n href=\"one\" \r\n id=\"two\" \f >");
    //
    ASSERT_EQ ("<a href=\"one\" id=\"two\"></a>", doc->to_html());
}

//test45
TEST(test, handlesWhitespaceInoDocType){
    string html = "<!DOCTYPE html\r\n PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\r\n \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">";
    parser parser;
    node_ptr doc = parser.parse(html);
    ASSERT_EQ ("<!DOCTYPE html\r\n PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\r\n \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">", doc->to_html());
}

//test46
TEST(test, testUsingSingleQuotesInQueries){
    string body = "<body> <div class='main'>hello</div></body>";
    parser parser;
    node_ptr doc = parser.parse(body);
    node_ptr main = doc->select("div[class='main']");

    ASSERT_EQ ("hello", main->to_text());
}

//test47
TEST(test, testSupportsNonAsciiTags){
    string body = "<a進捗推移グラフ>Yes</a進捗推移グラフ><bрусский-тэг>Correct</<bрусский-тэг>";
    parser parser;
    node_ptr doc = parser.parse(body);
    node_ptr els = doc->select("a進捗推移グラフ");
    ASSERT_EQ("Yes", els->to_text());
    els = doc->select("bрусский-тэг");
    ASSERT_EQ("Correct", els->to_text());
}

//test48
TEST(test, testSupportsPartiallyNonAsciiTags){
    string body = "<div>Check</divá>";
    parser parser;
    node_ptr doc = parser.parse(body);
    node_ptr els = doc->select("div");
    ASSERT_EQ("Check", els->to_text());
}

//test49
TEST(test, testHtmlLowerCaseAttributesForm){
    string html =  "<form NAME=one>";
    parser parser;
    node_ptr doc = parser.parse(html);
    ASSERT_EQ("<form name=\"one\"></form>", doc->to_html());
}

//test50
TEST(test, handlesControlCodeInAttributeName){
    parser parser;
    node_ptr doc = parser.parse("<p><a \06=foo>One</a><a/\06=bar><a foo\06=bar>Two</a></p>");
    // can not remove control code
    ASSERT_EQ("<p><a \x6=\"foo\">One</a><a \x6=\"bar\"><a foo\x6=\"bar\">Two</a></a></p>", doc->to_html());
}


GTEST_API_ int main(int argc, char ** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
