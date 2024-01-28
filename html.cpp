#include "html.hpp"

namespace html {

const std::unordered_set<std::string> inline_tags = {"b", "big", "i", "small", "tt",
	"abbr", "acronym", "cite", "code", "dfn", "em", "kbd", "strong", "samp",
	"time", "var", "a", "bdo", "br", "img", "map", "object", "q",
	"span", "sub", "sup", "button", "input", "label", "select", "textarea"};

const std::unordered_set<std::string> void_tags = {"area", "base", "br", "col", "embed",
	"hr", "img", "input", "link", "meta", "param", "source", "track", "wbr"};

const std::unordered_set<std::string> rawtext_tags = {"title", "textarea", "style", "script",
	"noscript", "plaintext", "iframe", "xmp", "noembed", "noframes"};

const std::string space_chars(" \f\n\r\t\v");

selector::selector(const std::string& s) {
	selector_matcher matcher;
	condition match_condition;
	char c = 0;
	bool reconsume = false;
	state_t state = state_t::tag;
	if(s == "*") {
		matcher.all_match = true;
		matchers.push_back(std::move(matcher));
		return;
	}
	auto it = s.begin();
	auto save_matcher = [&]() {
		if(!matcher.conditions.empty()) {
			matchers.push_back(std::move(matcher));
		}
	};
	auto save_cond = [&](const std::string& str) {
		if(!str.empty()) {
			if(matcher.conditions.empty()) {
				matcher.conditions.emplace_back();
			}
			matcher.conditions.back().push_back(std::move(match_condition));
		}
	};
	do {
		if(!reconsume) {
			c = it == s.end() ? 0 : *it++;
		}
		reconsume = false;
		switch(state) {
			case state_t::route:
				if(c == 0 || c == ' ') {
					save_matcher();
					state = state_t::tag;
				} else if(c == '>') {
					if(!matcher.dc_second) {
						matcher.dc_first = true;
					}
					save_matcher();
					matcher.dc_second = true;
					state = state_t::tag;
				} else if(c == '[') {
					state = state_t::attr;
				} else if(c == ':') {
					state = state_t::st_operator;
				} else if(c == '.') {
					state = state_t::st_class;
				} else if(c == '#') {
					state = state_t::id;
				} else if(c == ',') {
					matcher.conditions.emplace_back();
					state = state_t::tag;
				}
			break;
			case state_t::tag:
				if(is_state_route(c)) {
					save_cond(match_condition.tag_name);
					reconsume = true;
					state = state_t::route;
				} else if(utils::is_uppercase_alpha(c)) {
					match_condition.tag_name += std::tolower(c);
				} else {
					match_condition.tag_name += c;
				}
			break;
			case state_t::st_class:
				if(is_state_route(c)) {
					save_cond(match_condition.class_name);
					reconsume = true;
					state = state_t::route;
				} else {
					match_condition.class_name += c;
				}
			break;
			case state_t::id:
				if(is_state_route(c)) {
					save_cond(match_condition.id);
					reconsume = true;
					state = state_t::route;
				} else {
					match_condition.id += c;
				}
			break;
			case state_t::st_operator:
				if(is_state_route(c)) {
					save_cond(match_condition.attr_operator);
					reconsume = true;
					state = state_t::route;
				} else if(c == '(') {
					state = state_t::index;
				} else if(utils::is_uppercase_alpha(c)) {
					match_condition.attr_operator += std::tolower(c);
				} else {
					match_condition.attr_operator += c;
				}
			break;
			case state_t::index:
				if(c == ')') {
					save_cond(match_condition.index);
					state = state_t::route;
				} else if(utils::is_digit(c)) {
					match_condition.index += c;
				}
			break;
			case state_t::attr:
				if(c == ']') {
					save_cond(match_condition.attr);
					state = state_t::route;
				} else if(c == '=' || c == '*' || c == '^' || c == '$' || c == '!' || c == '~' || c == '|') {
					reconsume = true;
					state = state_t::attr_operator;
				} else if(utils::is_uppercase_alpha(c)) {
					match_condition.attr += std::tolower(c);
				} else {
					match_condition.attr += c;
				}
			break;
			case state_t::attr_operator:
				if(c == '\'') {
					state = state_t::attr_val;
				} else {
					match_condition.attr_operator += c;
				}
			break;
			case state_t::attr_val:
				if(c == '\'') {
					save_cond(match_condition.attr_operator);
					state = state_t::attr;
				} else {
					match_condition.attr_value += c;
				}
			break;
		}
	} while(c || reconsume);
}

selector::condition::condition(condition&& c) noexcept
	: tag_name(std::move(c.tag_name))
	, id(std::move(c.id))
	, class_name(std::move(c.class_name))
	, index(std::move(c.index))
	, attr(std::move(c.attr))
	, attr_value(std::move(c.attr_value))
	, attr_operator(std::move(c.attr_operator)) {
	c.tag_name.clear();
	c.id.clear();
	c.class_name.clear();
	c.index = "0";
	c.attr.clear();
	c.attr_value.clear();
	c.attr_operator.clear();
}

selector::selector_matcher::selector_matcher(selector_matcher&& m) noexcept
	: dc_first(m.dc_first)
	, dc_second(m.dc_second)
	, all_match(m.all_match)
	, conditions(std::move(m.conditions)) {
	m.all_match = false;
	m.dc_first = false;
	m.dc_second = false;
	m.conditions.clear();
}

bool selector::condition::operator()(const node& d) const {
	const int i = std::stoi(index);
	if(!tag_name.empty()) {
		return d.tag_name == tag_name;
	}
	if(!id.empty()) {
		auto it = d.attributes.find("id");
		if(it != d.attributes.end()) {
			return it->second == id;
		}
	}
	if(!class_name.empty()) {
		auto it = d.attributes.find("class");
		if(it != d.attributes.end()) {
			return utils::contains_word(it->second, class_name);
		}
	}
	if(attr_operator == "first") {
		return d.index == 0;
	}
	if(attr_operator == "last") {
		return d.index == d.parent->node_count - 1;
	}
	if(attr_operator == "eq") {
		return d.index == i;
	}
	if(attr_operator == "gt") {
		return d.index > i;
	}
	if(attr_operator == "lt") {
		return d.index < i;
	}
	if(!attr.empty()) {
		auto it = d.attributes.find(attr);
		if(it == d.attributes.end()) {
			return attr_operator == "!=";
		}
		if(attr_operator == "=") {
			return it->second == attr_value;
		} else if(attr_operator == "^=") {
			return it->second.find(attr_value) == 0;
		} else if(attr_operator == "$=") {
			return attr_value.size() <= it->second.size() && it->second.find(attr_value) == it->second.size() - attr_value.size();
		} else if(attr_operator == "!=") {
			return it->second != attr_value;
		} else if(attr_operator == "*=") {
			return it->second.find(attr_value) != std::string::npos;
		} else if(attr_operator == "~=") {
			return utils::contains_word(it->second, attr_value);
		} else if(attr_operator == "|=") {
			return it->second.find(attr_value) == 0 && 
				(attr_value.size() == it->second.size() || it->second[attr_value.size()] == '-');
		}
		return true;
	}
	return false;
}

bool selector::selector_matcher::operator()(const node& d) const {
	if(d.type_node != node_t::tag) {
		return false;
	}
	if(this->all_match) {
		return true;
	}
	for(auto& c : conditions) {
		size_t i = 0;
		for(; i < c.size(); i++) {
			if(!c[i](d)) {
				break;
			}
		}
		if(i == c.size()) {
			return true;
		}
	}
	return false;
}

node::node(const node& d)
	: type_node(d.type_node)
	, type_tag(d.type_tag)
	, self_closing(d.self_closing)
	, tag_name(d.tag_name)
	, content(d.content)
	, bogus_comment(d.bogus_comment)
	, attributes(d.attributes) {
	for(auto& n : d.children) {
		copy(n.get(), this);
	}
}

void node::walk(std::function<bool(node&)> handler) {
	walk(*this, handler);
}

void node::walk(node& d, std::function<bool(node&)> handler) {
	for(auto& c : d.children) {
		if(handler(*c)) {
			walk(*c, handler);
		}
	}
}

std::vector<node*> node::select(const selector s, bool nested) {
	std::vector<node*> matched_dom;
	size_t msize = s.matchers.size();
	if(msize) {
		matched_dom.push_back(this);
	}
	size_t i = 0;
	for(auto& matcher : s) {
		auto selectee_dom = std::move(matched_dom);
		for(auto p : selectee_dom) {
			walk(*p, [&](node& n) {
				if(matcher(n)) {
					matched_dom.push_back(&n);
					if(matcher.dc_second) {
						// div>[div]>div, div>div>[div] - not scan child, we need only direct child
						return false;
					} else if(matcher.dc_first) {
						// [div]>div>div - scan all child, since elements can be at any level
						return true;
					} else if(i < msize - 1) {
						// [div] div div, div [div] div - not scan child, the topmost parent will suffice
						return false;
					} else {
						// div div [div] - last matcher, scan based on attribute `nested`
						return nested;
					}
				} else if(matcher.dc_second) {
					return false;
				}
				// if not match and not direct child, scan all child
				return true;
			});
		}
		i++;
	}
	return matched_dom;
}

void node::to_html(std::ostream& out, bool child, bool text, int level, int& deep, char ind, bool& last_is_block, bool& sibling_is_block) const {
	std::streamoff pos = out.tellp();
	if(type_node == node_t::none) {
		for(auto& c : children) {
			c->to_html(out, child, text, 0, deep, ind, last_is_block, sibling_is_block);
		}
	} else if(type_node == node_t::text) {
		if(text && std::any_of(content.begin(), content.end(), [](char c) {
			return !utils::is_space(c);
		})) {
			auto str = content;
			if(parent && rawtext_tags.find(parent->tag_name) == rawtext_tags.end()) {
				str = utils::replace_any_copy(str, space_chars, " ");
			}
			if(last_is_block) {
				out << '\n' << std::string(deep, ind);
			}
			out << str;
			last_is_block = false;
		}
	} else if(type_node == node_t::tag) {
		bool old_is_block = last_is_block;
		last_is_block = inline_tags.find(tag_name) == inline_tags.end();
		if(pos && (old_is_block || last_is_block)) {
			out << '\n' << std::string(deep, ind);
			if(level && last_is_block && !sibling_is_block) {
				sibling_is_block = true;
				deep++;
				out << ind;
			}
		}
		out << "<" << tag_name;
		if(!attributes.empty()) {
			for(auto& a : attributes) {
				out << ' ' << a.first << "=\"" << a.second << "\"";
			}
		}
		if(self_closing) {
			out << " />";
		} else {
			out << ">";
			if(child) {
				bool last_is_block_n = false;
				bool sibling_is_block_n = false;
				for(auto& c : children) {
					c->to_html(out, child, text, level + 1, deep, ind, last_is_block_n, sibling_is_block_n);
				}
				if(sibling_is_block_n) {
					if(deep > 0) {
						deep--;
					}
					out << '\n' << std::string(deep, ind);
				}
			}
			out << "</" << tag_name << ">";
		}
	} else if(type_node == node_t::comment) {
		if(last_is_block) {
			out << '\n' << std::string(deep, ind);
		}
		out << "<!--" << content << "-->";
		last_is_block = false;
	} else if(type_node == node_t::doctype) {
		out << "<!DOCTYPE " << content << ">";
		last_is_block = true;
		sibling_is_block = true;
	}
}

void node::to_raw_html(std::ostream& out, bool child, bool text) const {
	if(type_node == node_t::none) {
		for(auto& c : children) {
			c->to_raw_html(out, child, text);
		}
	} else if(type_node == node_t::text) {
		if(text && std::any_of(content.begin(), content.end(), [](char c) {
			return !utils::is_space(c);
		})) {
			auto str = content;
			if(parent && rawtext_tags.find(parent->tag_name) == rawtext_tags.end()) {
				str = utils::replace_any_copy(str, space_chars, " ");
			}
			out << str;
		}
	} else if(type_node == node_t::tag) {
		out << "<" << tag_name;
		if(!attributes.empty()) {
			for(auto& a : attributes) {
				out << ' ' << a.first << "=\"" << a.second << "\"";
			}
		}
		if(self_closing) {
			out << " />";
		} else {
			out << ">";
			if(child) {
				for(auto& c : children) {
					c->to_raw_html(out, child, text);
				}
			}
			out << "</" << tag_name << ">";
		}
	} else if(type_node == node_t::comment) {
		out << "<!--" << content << "-->";
	} else if(type_node == node_t::doctype) {
		out << "<!DOCTYPE " << content << ">";
	}
}

std::string node::to_html(char ind, bool child, bool text) const {
	std::stringstream ret;
	bool last_is_block_n = false;
	bool sibling_is_block_n = false;
	int deep = 0;
	to_html(ret, child, text, 0, deep, ind, last_is_block_n, sibling_is_block_n);
	return ret.str();
}

std::string node::to_raw_html(bool child, bool text) const {
	std::stringstream ret;
	to_raw_html(ret, child, text);
	return ret.str();
}

void node::to_text(std::ostream& out, bool& is_block) const {
	std::streamoff pos = out.tellp();
	if(type_node == node_t::none) {
		for(auto& c : children) {
			c->to_text(out, is_block);
		}
	} else if(type_node == node_t::text) {
		if(is_block) {
			if(pos) {
				out << '\n';
			}
			is_block = false;
		}
		out << content;
	} else if(type_node == node_t::tag) {
		if(tag_name == "br") {
			out << '\n';
		}
		bool is_block_n = inline_tags.find(tag_name) == inline_tags.end();
		if(is_block_n) {
			is_block = true;
		}
		for(auto& c : children) {
			c->to_text(out, is_block);
		}
		if(is_block_n) {
			is_block = true;
		}
	}
}

std::string node::to_text(bool raw) const {
	std::stringstream ret;
	bool is_block = false;
	to_text(ret, is_block);
	auto str = ret.str();
	if(raw) {
		str = utils::replace_any_copy(str, space_chars, " ");
	}
	return str;
}

bool node::has_attr(const std::string& key) const {
	return attributes.count(key);
}

std::string node::get_attr(const std::string& attr) const {
	auto it = attributes.find(attr);
	if(it == attributes.end()) {
		return std::string();
	}
	return it->second;
}

void node::set_attr(const std::string& key, const std::string& val) {
	attributes[key] = val;
}

void node::set_attr(const std::map<std::string, std::string>& attr) {
	attributes = attr;
}

void node::del_attr(const std::string& key) {
	attributes.erase(key);
}

void node::copy(const node* n, node* p) {
	auto new_node = utils::make_unique<node>();
	new_node->parent = p;
	new_node->type_node = n->type_node;
	new_node->type_tag = n->type_tag;
	new_node->self_closing = n->self_closing;
	new_node->tag_name = n->tag_name;
	new_node->content = n->content;
	new_node->attributes = n->attributes;
	new_node->bogus_comment = n->bogus_comment;
	if(new_node->type_node == node_t::tag) {
		new_node->index = p->node_count++;
	}
	for(auto& c : n->children) {
		copy(c.get(), new_node.get());
	}
	p->children.push_back(std::move(new_node));
}

node& node::append(const node& n) {
	copy(&n, this);
	return *this;
}

void parser::operator()(node& nodeptr) {
	for(auto& c : callback_node) {
		if(!c.first) {
			c.second(nodeptr);
			continue;
		}
		auto it = c.first.begin();
		if((*it)(nodeptr)) {
			it++;
		}
		if(it == c.first.end()) {
			c.second(nodeptr);
		}
	}
}

parser& parser::set_callback(std::function<void(node&)> cb) {
	callback_node.push_back(std::make_pair(selector(), cb));
	return *this;
}

parser& parser::set_callback(const selector selector, std::function<void(node&)> cb) {
	callback_node.push_back(std::make_pair(selector, cb));
	return *this;
}

parser& parser::set_callback(std::function<void(err_t, node&)> cb) {
	callback_err.push_back(cb);
	return *this;
}

void parser::clear_callbacks() {
	callback_node.clear();
	callback_err.clear();
}

void parser::handle_node() {
	node* new_node_ptr = new_node.get();
	if(new_node_ptr->type_node == node_t::tag) {
		if(new_node_ptr->type_tag == tag_t::open) {
			new_node_ptr->index = current_ptr->node_count++;
			current_ptr->children.push_back(std::move(new_node));
			if(!new_node_ptr->self_closing) {
				if(void_tags.find(new_node_ptr->tag_name) != void_tags.end()) {
					new_node_ptr->self_closing = true;
				} else if(rawtext_tags.find(new_node_ptr->tag_name) != rawtext_tags.end()) {
					current_ptr = new_node_ptr;
					state = state_t::rawtext;
				} else {
					current_ptr = new_node_ptr;
				}
			}
			(*this)(*new_node_ptr);
		} else if(new_node_ptr->type_tag == tag_t::close) {
			auto _current_ptr = current_ptr;
			std::vector<node*> not_closed;
			while(_current_ptr->parent && _current_ptr->tag_name != new_node_ptr->tag_name) {
				not_closed.push_back(_current_ptr);
				_current_ptr = _current_ptr->parent;
			}
			if(_current_ptr->parent && _current_ptr->tag_name == new_node_ptr->tag_name) {
				for(auto& c : callback_err) {
					for(auto n : not_closed) {
						c(err_t::tag_not_closed, *n);
					}
				}
				if(!new_node_ptr->content.empty()) {
					auto text_node = utils::make_unique<node>(current_ptr);
					text_node->type_node = node_t::text;
					text_node->content = std::move(new_node_ptr->content);
					new_node_ptr->content.clear();
					current_ptr->children.push_back(std::move(text_node));
				}
				current_ptr = _current_ptr->parent;
				(*this)(*new_node_ptr);
			}
		}
	} else if(new_node_ptr->type_node == node_t::text) {
		if(!new_node_ptr->content.empty()) {
			current_ptr->children.push_back(std::move(new_node));
			(*this)(*new_node_ptr);
		}
	} else {
		current_ptr->children.push_back(std::move(new_node));
		(*this)(*new_node_ptr);
	}
	new_node = utils::make_unique<node>(current_ptr);
	new_node->type_node = node_t::text;
}

node_ptr html::parser::parse(const std::string& html) {
	return parser::parse(html.begin(), html.end());
}

node_ptr html::parser::parse(std::istream& html) {
	return parser::parse(std::istreambuf_iterator<char>(html), std::istreambuf_iterator<char>());
}

template<class InputIt>
node_ptr html::parser::parse(InputIt it, InputIt end) {
	char c = 0;
	bool reconsume = false;
	state = state_t::data;
	auto _parent = utils::make_unique<node>();
	current_ptr = _parent.get();
	new_node = utils::make_unique<node>(current_ptr);
	new_node->type_node = node_t::text;
	std::string k;
	while(it != end) {
		c = *it;
		switch(state) {
			case state_t::data: // 0
				if(c == '<') {
					state = state_t::tag_open;
				} else {
					new_node->content += c;
				}
			break;
			case state_t::rawtext: // 3
				if(c == '<') {
					state = state_t::rawtext_less_than_sign;
				} else if(c == 0x00) {
					new_node->content += '_';
				} else {
					new_node->content += c;
				}
			break;
			case state_t::tag_open: // 6
				if(c == '!') {
					state = state_t::markup_dec_open_state;
				} else if(c == '/') {
					state = state_t::end_tag_open;
				} else if(utils::is_alpha(c)) {
					state = state_t::tag_name;
					handle_node();
					new_node->type_node = node_t::tag;
					new_node->type_tag = tag_t::open;
					reconsume = true;
				} else if(c == '?') {
					state = state_t::bogus_comment;
					handle_node();
					new_node->type_node = node_t::comment;
					reconsume = true;
				} else {
					new_node->content += '<';
					reconsume = true;
					state = state_t::data;
				}
			break;
			case state_t::end_tag_open: // 7
				if(utils::is_alpha(c)) {
					state = state_t::tag_name;
					handle_node();
					new_node->type_node = node_t::tag;
					new_node->type_tag = tag_t::close;
					reconsume = true;
				} else if(c == '>') {
					state = state_t::data;
				} else {
					state = state_t::bogus_comment;
					handle_node();
					new_node->type_node = node_t::comment;
					reconsume = true;
				}
			break;
			case state_t::tag_name: // 8
				if(utils::is_space(c)) {
					state = state_t::before_attribute_name;
				} else if(c == '/') {
					state = state_t::self_closing;
				} else if(c == '>') {
					state = state_t::data;
					handle_node();
				} else if(utils::is_uppercase_alpha(c)) {
					new_node->tag_name += std::tolower(c);
				} else if(c == 0x00) {
					new_node->tag_name += '_';
				} else {
					new_node->tag_name += c;
				}
			break;
			case state_t::rawtext_less_than_sign: // 12
				if(c == '/') {
					state = state_t::rawtext_end_tag_open;
				} else {
					new_node->content += '<';
					reconsume = true;
					state = state_t::rawtext;
				}
			break;
			case state_t::rawtext_end_tag_open: // 13
				if(utils::is_alpha(c)) {
					new_node->type_node = node_t::tag;
					new_node->type_tag = tag_t::close;
					reconsume = true;
					state = state_t::rawtext_end_tag_name;
				} else {
					new_node->content += '<';
					new_node->content += '/';
					reconsume = true;
					state = state_t::rawtext;
				}
			break;
			case state_t::rawtext_end_tag_name: { // 14
				bool anything_else = true;
				if(utils::is_space(c)) {
					if(new_node->tag_name == current_ptr->tag_name) {
						state = state_t::before_attribute_name;
						anything_else = false;
					}
				} else if(c == '/') {
					if(new_node->tag_name == current_ptr->tag_name) {
						state = state_t::self_closing;
						anything_else = false;
					}
				} else if(c == '>') {
					if(new_node->tag_name == current_ptr->tag_name) {
						state = state_t::data;
						handle_node();
						anything_else = false;
					}
				} else if(utils::is_uppercase_alpha(c)) {
					new_node->tag_name += std::tolower(c);
					anything_else = false;
				} else if(utils::is_lowercase_alpha(c)) {
					new_node->tag_name += c;
					anything_else = false;
				}
				if(anything_else) {
					new_node->content += '<';
					new_node->content += '/';
					new_node->content += new_node->tag_name;
					new_node->tag_name.clear();
					reconsume = true;
					state = state_t::rawtext;
				}
			}
			break;
			case state_t::before_attribute_name: // 32
				if(utils::is_space(c)) {
					// skip
				} else if(c == '/' || c == '>') {
					reconsume = true;
					state = state_t::after_attribute_name;
				} else if(c == '=') {
					k = c;
					state = state_t::attribute_name;
				} else {
					k.clear();
					reconsume = true;
					state = state_t::attribute_name;
				}
			break;
			case state_t::attribute_name: // 33
				if(utils::is_space(c) || c == '/' || c == '>') {
					new_node->attributes[k];
					reconsume = true;
					state = state_t::after_attribute_name;
				} else if(c == '=') {
					new_node->attributes[k];
					state = state_t::before_attribute_value;
				} else if(c == 0x00) {
					k += '_';
				} else if(c == '\'' || c == '"' || c == '<') {
					k += c;
				} else {
					k += std::tolower(c);
				}
			break;
			case state_t::after_attribute_name: // 34
				if(utils::is_space(c)) {
					// skip
				} else if(c == '/') {
					state = state_t::self_closing;
				} else if(c == '=') {
					state = state_t::before_attribute_value;
				} else if(c == '>') {
					state = state_t::data;
					handle_node();
				} else {
					k.clear();
					reconsume = true;
					state = state_t::attribute_name;
				}
			break;
			case state_t::before_attribute_value: // 35
				if(utils::is_space(c)) {
					// skip
				} else if(c == '"') {
					state = state_t::attribute_value_double;
				} else if(c == '\'') {
					state = state_t::attribute_value_single;
				} else if(c == '>') {
					state = state_t::data;
					handle_node();
				} else {
					reconsume = true;
					state = state_t::attribute_value_unquoted;
				}
			break;
			case state_t::attribute_value_double: // 36
				if(c == '"') {
					state = state_t::after_attribute_value_quoted;
				} else if(c == 0x00) {
					new_node->attributes[k] += '_';
				} else {
					new_node->attributes[k] += c;
				}
			break;
			case state_t::attribute_value_single: // 37
				if(c == '\'') {
					state = state_t::after_attribute_value_quoted;
				} else if(c == 0x00) {
					new_node->attributes[k] += '_';
				} else {
					new_node->attributes[k] += c;
				}
			break;
			case state_t::attribute_value_unquoted: // 38
				if(utils::is_space(c)) {
					state = state_t::before_attribute_name;
				} else if(c == '>') {
					state = state_t::data;
					handle_node();
				} else if(c == 0x00) {
					new_node->attributes[k] += '_';
				} else if(c == '"' || c == '\'' || c == '<' || c == '=' || c == '`') {
					new_node->attributes[k] += c;
				} else {
					new_node->attributes[k] += c;
				}
			break;
			case state_t::after_attribute_value_quoted: // 39
				if(utils::is_space(c)) {
					state = state_t::before_attribute_name;
				} else if(c == '/') {
					state = state_t::self_closing;
				} else if(c == '>') {
					state = state_t::data;
					handle_node();
				} else {
					reconsume = true;
					state = state_t::before_attribute_name;
				}
			break;
			case state_t::self_closing: // 40
				if(c == '>') {
					new_node->self_closing = true;
					state = state_t::data;
					handle_node();
				} else {
					reconsume = true;
					state = state_t::before_attribute_name;
				}
			break;
			case state_t::bogus_comment: // 41
				if(c == '>') {
					state = state_t::data;
					handle_node();
				} else if(c == 0x00) {
					new_node->content += '_';
				} else {
					new_node->content += c;
				}
			break;
			case state_t::markup_dec_open_state: // 42
				if(utils::ilook_ahead(it, end, "--")) {
					std::advance(it, 2);
					state = state_t::comment_start;
					handle_node();
					new_node->type_node = node_t::comment;
					reconsume = true;
				} else if(utils::ilook_ahead(it, end, "DOCTYPE")) {
					std::advance(it, 7);
					state = state_t::before_doctype_name;
					handle_node();
					new_node->type_node = node_t::doctype;
					reconsume = true;
				} else {
					state = state_t::bogus_comment;
					handle_node();
					new_node->type_node = node_t::comment;
					new_node->bogus_comment = true;
					new_node->content += c;
				}
			break;
			case state_t::comment_start: // 43
				if(c == '-') {
					state = state_t::comment_start_dash;
				} else if(c == '>') {
					state = state_t::data;
					handle_node();
				} else {
					reconsume = true;
					state = state_t::comment;
				}
			break;
			case state_t::comment_start_dash: // 44
				if(c == '-') {
					state = state_t::comment_end;
				} else if(c == '>') {
					state = state_t::data;
					handle_node();
				} else {
					new_node->content += '-';
					state = state_t::comment;
				}
			break;
			case state_t::comment: // 45
				if(c == '-') {
					state = state_t::comment_end_dash;
				} else if(c == 0x00) {
					new_node->content += '_';
				} else {
					new_node->content += c;
				}
			break;
			case state_t::comment_end_dash: // 50
				if(c == '-') {
					state = state_t::comment_end;
				} else {
					new_node->content += '-';
					state = state_t::comment;
				}
			break;
			case state_t::comment_end: // 51
				if(c == '>') {
					state = state_t::data;
					handle_node();
				} else if(c == '-') {
					new_node->content += c;
				} else {
					new_node->content += '-';
					new_node->content += '-';
					reconsume = true;
					state = state_t::comment;
				}
			break;
			case state_t::before_doctype_name: // 54
				if(utils::is_space(c)) {
					// skip
				} else if(c == '>') {
					state = state_t::data;
					handle_node();
				} else if(c == 0x00) {
					new_node->content += '_';
					state = state_t::doctype_name;
				} else {
					new_node->content += c;
					state = state_t::doctype_name;
				}
			break;
			case state_t::doctype_name: // 55
				if(c == '>') {
					state = state_t::data;
					handle_node();
				} else if(c == 0x00) {
					new_node->content += '_';
				} else {
					new_node->content += c;
				}
			break;
		}
		if(!reconsume) {
			it++;
		} else {
			reconsume = false;
		}
	}
	new_node->type_node = node_t::text;
	handle_node();
	return _parent;
}

node utils::make_node(node_t type, const std::string& str, const std::map<std::string, std::string>& attributes) {
	html::node node;
	node.type_node = type;
	if(type == node_t::tag) {
		node.tag_name = str;
		if(void_tags.find(str) != void_tags.end()) {
			node.self_closing = true;
		}
		if(!attributes.empty()) {
			node.set_attr(attributes);
		}
	} else {
		node.content = str;
	}
	return node;
}

inline bool utils::contains_word(const std::string& str, const std::string& word) {
	auto pos = str.find(word);
	if(pos == std::string::npos) {
		return false;
	}
	bool start = pos < 1 || is_space(str[pos - 1]);
	bool end = pos + word.size() >= str.size() || is_space(str[pos + word.size()]);
	return start && end;
}

template<class InputIt>
inline bool utils::ilook_ahead(InputIt it, InputIt end, const std::string& str) {
	for(std::string::size_type i = 0; i < str.size(); i++, it++) {
		if(it == end || std::tolower(str[i]) != std::tolower(*it)) {
			return false;
		}
	}
	return true;
}

std::string utils::replace_any_copy(const std::string& subject, const std::string& search, const std::string& replace) {
    size_t pos = 0, prev = 0;
    std::string ret;
    while((pos = subject.find_first_of(search, pos)) != std::string::npos) {
        if(pos > prev || !pos) {
            ret.append(subject, prev, pos - prev);
            ret.append(replace);
        }
        prev = ++pos;
    }
    ret.append(subject, prev, std::string::npos);
    return ret;
}

}