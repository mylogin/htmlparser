## Usage

### Access nodes
```cpp
html::parser p;
html::node_ptr node = p.parse(R"(<!DOCTYPE html><body><div attr="val">text</div><!--comment--></body>)");

// `parse` method returns root node of type html::node_t::none
assert(node->type_node == html::node_t::none);
assert(node->at(0)->type_node == html::node_t::doctype);
assert(node->at(1)->type_node == html::node_t::tag);
assert(node->at(1)->at(0)->at(0)->type_node == html::node_t::text);
assert(node->at(1)->at(1)->type_node == html::node_t::comment);

std::cout << "Number of child elements: " << node->size() << std::endl << std::endl; // 2

std::cout << "Loop through child nodes: " << std::endl;
for(auto& n : *(node->at(1))) {
	std::cout << n->to_html() << std::endl;
}
std::cout << std::endl;

std::cout << "Get node properties: " << std::endl;
std::cout << "DOCTYPE name: " << node->at(0)->content << std::endl; // html
std::cout << "BODY tag: " << node->at(1)->tag_name << std::endl; // body
std::cout << "Attr value: " << node->at(1)->at(0)->get_attr("attr") << std::endl; // val
std::cout << "Text node: " << node->at(1)->at(0)->at(0)->content << std::endl; // text
std::cout << "Comment: " << node->at(1)->at(1)->content << std::endl; // comment
```

### Find nodes using `select` method
[List of available selectors](#selectors)
```cpp
html::parser p;
html::node_ptr node = p.parse(R"(<div id="my_id"><p class="my_class"></p></div>)");
std::vector<html::node*> selected = node->select("div#my_id p.my_class");
for(auto elem : selected) {
	std::cout << elem->to_html() << std::endl;
}
```

### Access nodes using callback (called when the document is parsed)
```cpp
html::parser p;
p.set_callback("meta[http-equiv='Content-Type'][content*='charset=']", [](html::node& n) {
	if (n.type_node == html::node_t::tag && n.type_tag == html::tag_t::open) {
		std::cout << "Callback with selector to filter elements:" << std::endl;
		std::cout << n.to_html() << std::endl << std::endl;
	}
});
p.set_callback([](html::node& n) {
	if(n.type_node == html::node_t::tag && n.type_tag == html::tag_t::open && n.tag_name == "meta") {
		if(n.get_attr("http-equiv") == "Content-Type" && n.get_attr("content").find("charset=") != std::string::npos) {
			std::cout << "Callback without selector:" << std::endl;
			std::cout << n.to_html() << std::endl;
		}
	}
});
p.parse(R"(<head><title>Title</title><meta http-equiv="Content-Type" content="text/html; charset=utf-8" /></head>)");
```

### Manual search
```cpp
std::cout << "Search `li` tags which not in `ol`:" << std::endl;
html::parser p;
html::node_ptr node = p.parse("<ul><li>li1</li><li>li2</li></ul><ol><li>li</li></ol>");
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
		std::cout << "Tag not closed: " << n.to_html(' ', false);
		std::string msg;
		html::node* current = &n;
		while(current->get_parent()) {
			msg.insert(0, " " + current->tag_name);
			current = current->get_parent();
		}
		msg.insert(0, "\nPath:");
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

std::cout << "Print text with line breaks preserved:" << std::endl;
std::cout << node->to_text() << std::endl << std::endl;

std::cout << "Print text with line breaks replaced with spaces:" << std::endl;
std::cout << node->to_text(true) << std::endl;
```

### Build document
```cpp
std::cout << "Using helpers:" << std::endl;

html::node hdiv = html::utils::make_node(html::node_t::tag, "div");
hdiv.append(html::utils::make_node(html::node_t::text, "Link:"));
hdiv.append(html::utils::make_node(html::node_t::tag, "br"));
html::node ha = html::utils::make_node(html::node_t::tag, "a", {{"href", "https://github.com/"}, {"class", "a_class"}});
ha.append(html::utils::make_node(html::node_t::text, "Github.com"));
std::cout << hdiv.append(ha).to_html() << std::endl << std::endl;

std::cout << "Without helpers:" << std::endl;

html::node div;
div.type_node = html::node_t::tag;
div.tag_name = "div";

html::node text;
text.type_node = html::node_t::text;
text.content = "Link:";
div.append(text);

html::node br;
br.type_node = html::node_t::tag;
br.tag_name = "br";
br.self_closing = true;
div.append(br);

html::node a;
a.type_node = html::node_t::tag;
a.tag_name = "a";
a.set_attr("href", "https://github.com/");
a.set_attr("class", "a_class");

html::node a_text;
a_text.type_node = html::node_t::text;
a_text.content = "Github.com";
a.append(a_text);

div.append(a);

std::cout << div.to_html() << std::endl;
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
| [attr] | element that have attribute "attr" | √ | √ |
| [attr='val'] | attribute is equal to "val" | √ | √ |
| [attr!='val'] | attribute is not equal to "val" or does not exist | √ | √ |
| [attr^='http:'] | attribute starts with "http:" | √ | √ |
| [attr$='.jpeg'] | attribute ends with ".jpeg" | √ | √ |
| [attr*='/path/'] | attribute contains "/path/" | √ | √ |
| div#id1.class1[attr='val'] | element that matches all of these selectors | √ | √ |
| p,div | element that matches any of these selectors | √ | √ |
| div p | all <p> elements inside <div> elements | √ | - |
| div>p | all <p> elements where the parent is a <div> element | √ | - |
| div div>p>i | combination of nested selectors  | √ | - |