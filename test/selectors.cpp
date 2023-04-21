#include "html.hpp"
#include <fstream>
#include <iostream>

struct sel {
	std::string name;
	std::string selector;
	bool callback;
};

int main(int argc, char* argv[]) {

	std::ifstream ifs("selectors.html");
	if(!ifs.is_open()) {
		std::cout << "specify html file\n";
		std::cin.get();
		return 0;
	}
	std::string page((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	ifs.close();

	std::vector<sel> selectors = {
		{"tag name", "p", true},
		{"id", "#id1", true},
		{"class", ".class_name", true},
		{"first element", ":first", true},
		{"last element", ":last", false},
		{"specified index", ":eq(1)", true},
		{"greater than", ":gt(1)", true},
		{"less than", ":lt(1)", true},
		{"attr exist", "[attr]", true},
		{"attr equal", "[attr='attr_val2']", true},
		{"attr not equal", "[attr!='attr_val2']", true},
		{"attr start with", "[attr^='attr']", true},
		{"attr ends with", "[attr$='val1']", true},
		{"attr contains", "[attr2*='alu']", true},
		{"all of selectors", "div#id2.class_name:last:eq(2)[attr2^='val'][attr2$='ue']", true},
		{"any of selectors", "p,div", true},
		{"nested selectors", "p[attr='attr_val1'] i", false}
	};

	html::parser p;
	
	std::cout << "Using 'select' method:\n\n";
	html::node_ptr n = p.parse(page);
	for(auto& s : selectors) {
		std::cout << s.name << " (" << s.selector << "):\n" << n->select(s.selector)->to_html() << "\n\n";
		if(!s.callback) {
			continue;
		}
		p.set_callback(s.selector, [s](html::node& n) {
			if(n.type_node == html::node_t::tag && n.type_tag == html::tag_t::open) {
				std::cout << s.name << " (" << s.selector << "):\n" << n.to_html() << "\n\n";
			}
		});
	}

	std::cout << "\nUsing callbacks:\n\n";
	n = p.parse(page);

	return 0;

}