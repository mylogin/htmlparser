#include <iostream>
#include <gtest/gtest.h>
#include <string>
#include "html.hpp"

using namespace std;
using namespace html;

//test11
TEST(test, dropsUnterminatedAttribute){
    string h1 = "<p id=\"foo";
    parser parse;
    node_ptr doc = parse.parse(h1);

    ASSERT_EQ("", doc->to_text());
}

//test12
TEST(test, testSpaceAfterTag){
    parser parse;
    node_ptr doc = parse.parse("<div > <a name=\"top\"></a ><p id=1 >Hello</p></div>");

    ASSERT_EQ("<div><a name=\"top\"></a>\n\t<p id=\"1\">Hello</p>\n</div>", doc->to_html());
}

//test13
TEST(test, createsStructureFromBodySnippet){
    string html = "foo <b>bar</b> baz";
    parser parse;
    node_ptr doc = parse.parse(html);

    ASSERT_EQ("foo bar baz", doc->to_text());
}

//test14
TEST(test, handlesTextAfterData){
    string h = "<html><body>pre <script>inner</script> aft</body></html>";
    parser parse;
    node_ptr doc = parse.parse(h);
    //<html><head></head><body>pre <script>inner</script> aft</body></html>
    // no head
    ASSERT_EQ("<html>\n\t<body>pre \n\t\t<script>inner</script>\n\t\t aft\n\t</body>\n</html>", doc->to_html());
}

//test15
TEST(test, handlesTextArea){
    parser parse;
    node_ptr doc = parse.parse("<textarea>Hello</textarea>");
    node_ptr els = doc->select("textarea");
    ASSERT_EQ("Hello", els->to_text());
}

//test16
TEST(test, discardsNakedTds){
    string h = "<td>Hello<td><p>There<p>now";
    parser parse;
    node_ptr doc = parse.parse(h);
    ASSERT_EQ("<td>Hello\n\t<td>\n\t\t<p>There\n\t\t\t<p>now</p>\n\t\t</p>\n\t</td>\n</td>", doc->to_html());
}

//test17
TEST(test, handlesNestedImplicitTable){
    parser parse;
    node_ptr doc = parse.parse("<table><td>1</td></tr> <td>2</td></tr> <td> <table><td>3</td> <td>4</td></table> <tr><td>5</table>");
    //<table><tbody><tr><td>1</td></tr><tr><td>2</td></tr><tr><td><table><tbody><tr><td>3</td><td>4</td></tr></tbody></table></td></tr><tr><td>5</td></tr></tbody></table>
    // no tbody
    ASSERT_EQ("<table>\n\t<td>1</td>\n\t<td>2</td>\n\t<td>\n\t\t<table>\n\t\t\t<td>3</td>\n\t\t\t<td>4</td>\n\t\t</table>\n\t\t<tr>\n\t\t\t<td>5</td>\n\t\t</tr>\n\t</td>\n</table>", doc->to_html());
}

//test18
TEST(test, handlesImplicitCaptionClose){
    parser parse;
    node_ptr doc = parse.parse("<table><caption>A caption<td>One<td>Two");
    // <table><caption>A caption</caption><tbody><tr><td>One</td><td>Two</td></tr></tbody></table>
    ASSERT_EQ("<table>\n\t<caption>A caption\n\t\t<td>One\n\t\t\t<td>Two</td>\n\t\t</td>\n\t</caption>\n</table>", doc->to_html());
}

//test19
TEST(test, handlesUnclosedCdataAtEOF){
    string h = "<![CDATA[]]";
    parser parse;
    node_ptr doc = parse.parse(h);
    ASSERT_EQ(1, doc->get_children().size());
}

//test20
TEST(test, handlesInvalidStartTags){
    string h = "<div>Hello < There <&amp;></div>";
    parser parse;
    node_ptr doc = parse.parse(h);
    // cannot remove amp;
    ASSERT_EQ("Hello < There <&amp;>", doc->select("div")->get_children()[0]->to_text());
}


GTEST_API_ int main(int argc, char ** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
