#include <iostream>
#include <gtest/gtest.h>
#include <string>
#include "html.hpp"

using namespace std;
using namespace html;

//test61
TEST(test, testKeepsPreText){
    string h = "<p>Hello \n \n there.</p> <div><pre>  What's \n\n  that?</pre>";
    parser parser;
    node_ptr doc = parser.parse(h);
    ASSERT_EQ("Hello \n \n there.\n \n  What's \n\n  that?", doc->to_text());
}

//test62
TEST(test, testKeepsPreTextInCode){
    string h = "<pre><code>code\n\ncode</code></pre>";
    parser parser;
    node_ptr doc = parser.parse(h);
    ASSERT_EQ("code\n\ncode", doc->to_text());
    ASSERT_EQ("<pre><code>code code</code></pre>", doc->to_html());
}

//test63
TEST(test, testKeepsPreTextAtDepth){
    string h = "<pre><code><span><b>code\n\ncode</b></span></code></pre>";
    parser parser;
    node_ptr doc = parser.parse(h);
    ASSERT_EQ("code\n\ncode", doc->to_text());
    ASSERT_EQ("<pre><code><span><b>code code</b></span></code></pre>", doc->to_html());
}

//test64
TEST(test, textHasSpacesAfterBlock){
    parser parser;
    node_ptr doc = parser.parse("<div>One</div><div>Two</div><span>Three</span><p>Fou<i>r</i></p>");
    string text = doc->to_text();

    ASSERT_EQ("One\nTwo\nThree\nFour", text);
    ASSERT_EQ("OneTwo", parser.parse("<span>One</span><span>Two</span>")->to_text());
}

//test65
TEST(test, testPrettyWithEnDashBody){
    string html = "<div><span>1:15</span>&ndash;<span>2:15</span>&nbsp;p.m.</div>";
    parser parser;
    node_ptr doc = parser.parse(html);

    ASSERT_EQ("<div><span>1:15</span>&ndash;<span>2:15</span>&nbsp;p.m.</div>", doc->to_html());
}

//test66
TEST(test, testBasicFormats){
    string html = "<span>0</span>.<div><span>1</span>-<span>2</span><p><span>3</span>-<span>4</span><div>5</div>";
    parser parser;
    node_ptr doc = parser.parse(html);


    ASSERT_EQ("<span>0</span>.\n<div><span>1</span>-<span>2</span>\n\t<p><span>3</span>-<span>4</span>\n\t\t<div>5</div>\n\t</p>\n</div>", doc->to_html());
}

//test67
TEST(test, textHasSpaceAfterBlockTags){
    parser parser;
    node_ptr doc = parser.parse("<div>One</div>Two");

    ASSERT_EQ("One\nTwo", doc->to_text());
}

//test68
TEST(test, textHasSpaceBetweenDivAndCenterTags){
    parser parser;
    node_ptr doc = parser.parse("<div>One</div><div>Two</div><center>Three</center><center>Four</center>");
    ASSERT_EQ("One\nTwo\nThree\nFour", doc->to_text());
}

//test69
TEST(test, wrapTextAfterBr){
    parser parser;
    node_ptr doc = parser.parse("<p>Hello<br>there<br>now.</p>");
    ASSERT_EQ("<p>Hello<br />there<br />now.</p>", doc->to_html());
}

//test70
TEST(test, prettyprintBrInBlock){
    string html = "<div><br> </div>";
    parser parser;
    node_ptr doc = parser.parse(html);
    ASSERT_EQ("<div><br /></div>", doc->to_html());
}


GTEST_API_ int main(int argc, char ** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
