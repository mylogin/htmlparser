#include "html.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
	
	std::ifstream ifs("test/example.html");
	if(!ifs.is_open()) {
		std::cout << "specify html file\n";
		std::cin.get();
		return 0;
	}
	std::string page((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	ifs.close();
	
	html::parser p;
	
	std::cout << "\n1. Callbacks: (called when the document is parsed):";
	p.set_callback("meta[http-equiv='Content-Type'][content*='charset=']", [](html::node& n) {
		if (n.type_node == html::node_t::tag && n.type_tag == html::tag_t::open) {
			std::cout << "\n\nCallback function with selector:\n";
			std::cout << n.to_html();
		}
	});
	p.set_callback([](html::node& n) {
		if(n.type_node == html::node_t::tag && n.type_tag == html::tag_t::open && n.tag_name == "meta") {
			if(n.get_attr("http-equiv") == "Content-Type" && n.get_attr("content").find("charset=") != std::string::npos) {
				std::cout << "\n\nCallback function without selector:\n";
				std::cout << n.to_html();
			}
		}
	});
	p.set_callback([](html::err_t e, html::node& n) {
		if(e == html::err_t::tag_not_closed) {
			std::cout << "\n\nCallback function to handle errors:";
			std::cout << "\nTag not closed: " << n.to_html(' ', false);
			std::string msg;
			html::node* current = &n;
			while(current->get_parent()) {
				msg.insert(0, " " + current->tag_name);
				current = current->get_parent();
			}
			msg.insert(0, "\nPath:");
			std::cout << msg;
		}
	});
	html::node_ptr n = p.parse(page);
	p.clear_callbacks();
	
	std::cout << "\n\n2. `select` method (scans already parsed document):\n";
	auto selected = n->select("html body h1#my_h1");
	std::cout << selected->to_html() << "\n";
	for (auto& elem : selected->children) {
		std::cout << "Tag: " << elem->tag_name << "\n";
		std::cout << "Attr: " << elem->get_attr("id");
	}
	
	std::cout << "\n\n3. Manual search in an already parsed document:\n";
	std::cout << "search `li` tags which not in `ol`:";
	n->walk([](html::node& n) {
		if(n.type_node == html::node_t::tag && n.tag_name == "ol") {
			return false; // not scan child tags
		}
		if(n.type_node == html::node_t::tag && n.tag_name == "li") {
			std::cout << "\n" << n.to_html();
		}
		return true; // scan child tags
	});
	
	std::cout << "\n\n4. Print document formatted:\n";
	std::cout << n->to_html(' ');
	
	std::cout << "\n\n5. Build document:\n";
	
	auto div = std::make_shared<html::node>();
	div->type_node = html::node_t::tag;
	div->tag_name = "div";
	
	auto text = std::make_shared<html::node>();
	text->type_node = html::node_t::text;
	text->content = "Link:";
	div->append(text);
	
	auto br = std::make_shared<html::node>();
	br->type_node = html::node_t::tag;
	br->tag_name = "br";
	br->self_closing = true;
	div->append(br);
	
	auto a = std::make_shared<html::node>();
	a->type_node = html::node_t::tag;
	a->tag_name = "a";
	a->set_attr("href", "https://github.com/");
	div->append(a);
	
	auto a_text = std::make_shared<html::node>();
	a_text->type_node = html::node_t::text;
	a_text->content = "Github.com";
	a->append(a_text);
	
	std::cout << div->to_html();
	std::cout << "\n";

	return 0;
		
}

