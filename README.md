## Usage

### Direct access to nodes
```cpp
html::parser p;
html::node_ptr node = p.parse(R"(<!DOCTYPE html><div attr="val">text</div><!--comment-->)");
// `parse` method returns root node of type html::node_t::none
assert(node->type_node == html::node_t::none);
assert(node->at(0)->type_node == html::node_t::doctype);
assert(node->at(1)->type_node == html::node_t::tag);
assert(node->at(1)->at(0)->type_node == html::node_t::text);
assert(node->at(2)->type_node == html::node_t::comment);
std::cout << "Number of children elements: " << node->size() << std::endl; // 3
std::cout << "DOCTYPE: " << node->at(0)->content << std::endl; // html
std::cout << "Tag name: " << node->at(1)->tag_name << std::endl; // div
std::cout << "Attr value: " << node->at(1)->get_attr("attr") << std::endl; // val
std::cout << "Text node: " << node->at(1)->at(0)->content << std::endl; // text
std::cout << "Comment: " << node->at(2)->content << std::endl; // comment
std::cout << "Print all: " << std::endl << node->to_html() << std::endl;
```

### Access nodes using `select` method
```cpp
html::parser p;
html::node_ptr node = p.parse(R"(<div id="my_id"><p class="my_class"></p></div>)");
std::cout << node->select("div#my_id p.my_class")->to_html() << std::endl;
```

### Access nodes using callbacks (called when parsing HTML)
```cpp
html::parser p;

// Callback with selector to filter elements
p.set_callback("meta[http-equiv='Content-Type'][content*='charset=']", [](html::node& n) {
	if (n.type_node == html::node_t::tag && n.type_tag == html::tag_t::open) {
		std::cout << "Meta charset (with selector): " << n.get_attr("content") << std::endl;
	}
});

// Callback without selector
p.set_callback([](html::node& n) {
	if(n.type_node == html::node_t::tag && n.type_tag == html::tag_t::open && n.tag_name == "meta") {
		if(n.get_attr("http-equiv") == "Content-Type" && n.get_attr("content").find("charset=") != std::string::npos) {
			std::cout << "Meta charset: " << n.get_attr("content") << std::endl;
		}
	}
});

p.parse(R"(<head><title>Title</title><meta http-equiv="Content-Type" content="text/html; charset=utf-8" /></head>)");
```

### Access nodes manually
```cpp
html::parser p;
html::node_ptr node = p.parse("<ul><li>li1</li><li>li2</li></ul><ol><li>li</li></ol>");

// Search `li` tags which not in `ol`
node->walk([](html::node& n) {
	if(n.type_node == html::node_t::tag && n.tag_name == "ol") {
		return false; // not scan child tags
	}
	if(n.type_node == html::node_t::tag && n.tag_name == "li") {
		std::cout << n.to_html() << std::endl;
	}
	return true; // scan child tags
});
```

### Finding unclosed tags
```cpp
html::parser p;

// Callback to handle errors
p.set_callback([](html::err_t e, html::node& n) {
	if(e == html::err_t::tag_not_closed) {
		std::string msg;
		html::node* current = &n;
		while(current->get_parent()) {
			msg.insert(0, " " + current->tag_name);
			current = current->get_parent();
		}
		msg.insert(0, "Unclosed tag:");
		std::cout << msg << std::endl;
	}
});

p.parse("<div><p><a></p></div>");
```

### Print document formatted
```cpp
html::parser p;
html::node_ptr node = p.parse("<ul><li>li1</li><li>li2</li></ul><ol><li>li</li></ol>");

// method takes two arguments, the indentation character and whether to output child elements (tabulation and true by default)
std::cout << node->to_html(' ', true) << std::endl;
```

### Print text content of a node
```cpp
html::parser p;
html::node_ptr node = p.parse("<div><p><b>First</b> p</p><p><i>Second</i> p</p>Text<br />Text</div>");

// print text with line breaks preserved
std::cout << node->to_text() << std::endl << std::endl;
// print text with line breaks replaced with spaces
std::cout << node->to_text(true) << std::endl;
```

### Building HTML from DOM tree
```cpp
auto p = html::utils::make_node(html::node_t::tag, "p", {{"id", "p_id"}, {"class", "p_class"}});
p->append(html::utils::make_node(html::node_t::text, "Link:"));
p->append(html::utils::make_node(html::node_t::tag, "br"));
auto a = html::utils::make_node(html::node_t::tag, "a", {{"href", "https://github.com/"}});
p->append(a);
a->append(html::utils::make_node(html::node_t::text, "Github.com"));
std::cout << html::utils::make_node(html::node_t::tag, "div")->append(p)->to_html() << std::endl;
```

## Selectors
| Selector example | Description | select | callback |
|-|-|-|-|
| * | all elements | √ | √ |
| div | tag name | √ | √ |
| #id1 | id="id1" | √ | √ |
| .class1 | class="class1" | √ | √ |
| :first | first element | √ | √ |
| :last | last element | √ | - |
| :eq(3) | element index = 3 (starts from 0) | √ | √ |
| :gt(3) | element index > 3 (starts from 0) | √ | √ |
| :lt(3) | element index < 3 (starts from 0) | √ | √ |
| [attr='val'] | attribute is equal to "val" | √ | √ |
| [attr!='val'] | attribute is not equal to "val" or does not exist | √ | √ |
| [attr^='http:'] | attribute starts with "http:" | √ | √ |
| [attr$='.jpeg'] | attribute ends with ".jpeg" | √ | √ |
| [attr*='/path/'] | attribute contains "/path/" | √ | √ |
| div#id1.class1[attr='val'] | element that matches all of these selectors | √ | √ |
| p,div | element that matches any of these selectors | √ | √ |
| div p i:first | nested selectors | √ | - |