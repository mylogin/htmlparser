#include "html.hpp"

namespace html {

std::vector<std::string> inline_tags = {"b", "big", "i", "small", "tt",
	"abbr", "acronym", "cite", "code", "dfn", "em", "kbd", "strong", "samp",
	"time", "var", "a", "bdo", "br", "img", "map", "object", "q",
	"span", "sub", "sup", "button", "input", "label", "select", "textarea"};

selector::selector(std::string s) {
	selector_matcher matcher;
	condition match_condition;
	char c;
	bool reconsume = false;
	int state = SEL_STATE_TAG;
	if(s == "*") {
		matcher.all_match = true;
		matchers.push_back(std::move(matcher));
		return;
	}
	auto it = s.begin();
	auto getc = [&]() -> char {
		if(it != s.end()) {
			return *it++;
		}
		return 0;
	};
	auto save_matcher = [&]() {
		if(!matcher.conditions.empty()) {
			matchers.push_back(std::move(matcher));
		}
	};
	auto save_cond = [&](const std::string& str) {
		if(!str.empty()) {
			matcher.conditions.push_back(std::move(match_condition));
		}
	};
	do {
		if(!reconsume) {
			c = getc();
		}
		reconsume = false;
		switch(state) {
			case SEL_STATE_TAG:
				if(c == 0 || c == ' ') {
					save_cond(match_condition.tag_name);
					save_matcher();
				} else if(c == '[') {
					save_cond(match_condition.tag_name);
					state = SEL_STATE_ATTR;
				} else if(c == ':') {
					save_cond(match_condition.tag_name);
					state = SEL_STATE_OPERATOR;
				} else if(c == '.') {
					save_cond(match_condition.tag_name);
					state = SEL_STATE_CLASS;
				} else if(c == '#') {
					save_cond(match_condition.tag_name);
					state = SEL_STATE_ID;
				} else {
					match_condition.tag_name += c;
				}
			break;
			case SEL_STATE_CLASS:
				if(c == 0 || c == ' ' || c == '[' || c == ':' || c == '.' || c == '#') {
					save_cond(match_condition.class_name);
					reconsume = true;
					state = SEL_STATE_TAG;
				} else {
					match_condition.class_name += c;
				}
			break;
			case SEL_STATE_ID:
				if(c == 0 || c == ' ' || c == '[' || c == ':' || c == '.' || c == '#') {
					save_cond(match_condition.id);
					reconsume = true;
					state = SEL_STATE_TAG;
				} else {
					match_condition.id += c;
				}
			break;
			case SEL_STATE_OPERATOR:
				if(c == 0 || c == ' ' || c == '[' || c == ':' || c == '.' || c == '#') {
					save_cond(match_condition.attr_operator);
					reconsume = true;
					state = SEL_STATE_TAG;
				} else if(c == '(') {
					save_cond(match_condition.attr_operator);
					state = SEL_STATE_INDEX;
				} else {
					match_condition.attr_operator += c;
				}
			break;
			case SEL_STATE_INDEX:
				if(c == ')') {
					save_cond(match_condition.index);
					state = SEL_STATE_TAG;
				} else if(std::isdigit(c)) {
					match_condition.index += c;
				}
			break;
			case SEL_STATE_ATTR:
				if(c == ']') {
					save_cond(match_condition.attr);
					state = SEL_STATE_TAG;
				} else if(c == '=' || c == '*' || c == '^' || c == '$' || c == '!') {
					reconsume = true;
					state = SEL_STATE_ATTR_OPERATOR;
				} else {
					match_condition.attr += c;
				}
			break;
			case SEL_STATE_ATTR_OPERATOR:
				if(c == '\'') {
					state = SEL_STATE_ATTR_VAL;
				} else {
					match_condition.attr_operator += c;
				}
			break;
			case SEL_STATE_ATTR_VAL:
				if(c == '\'') {
					save_cond(match_condition.attr_operator);
					state = SEL_STATE_ATTR;
				} else {
					match_condition.attr_value += c;
				}
			break;
		}
	} while(c || reconsume);
}

selector::condition::condition(condition&& c)
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
	c.index.clear();
	c.attr.clear();
	c.attr_value.clear();
	c.attr_operator.clear();
}

selector::selector_matcher::selector_matcher(selector_matcher&& m)
	: all_match(m.all_match)
	, conditions(std::move(m.conditions)) {
	m.all_match = false;
	m.conditions.clear();
}

bool selector::condition::operator()(const node& d) const {
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
			return it->second == class_name;
		}
	}
	if(attr_operator == "first") {
		return d.index == 0;
	}
	if(attr_operator == "last") {
		return d.index == d.parent->node_count - 1;
	}
	if(attr_operator == "eq") {
		return d.index == std::stoi(index);
	}
	if(attr_operator == "qt") {
		return d.index > std::stoi(index);
	}
	if(attr_operator == "lt") {
		return d.index < std::stoi(index);
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
			return it->second.find(attr_value) == attr_value.size() - 1;
		} else if(attr_operator == "!=") {
			return it->second != attr_value;
		} else if(attr_operator == "*=") {
			return it->second.find(attr_value) != std::string::npos;
		}
		return true;
	}
	return false;
}

bool selector::selector_matcher::operator()(const node& d) const {
	if(this->all_match) {
		return true;
	}
	for(auto& c : conditions) {
		if(c(d)) {
			continue;
		}
		return false;
	}
	return true;
}

node::node(node&& d)
	: tag_name(std::move(d.tag_name))
	, content_text(std::move(d.content_text))
	, attributes(std::move(d.attributes))
	, parent(std::move(d.parent))
	, children(std::move(d.children))
	, index(d.index)
	, node_count(d.node_count) {
	d.tag_name.clear();
	d.attributes.clear();
	d.content_text.clear();
	d.parent = nullptr;
	d.children.clear();
	d.index = 0;
	d.node_count = 0;
}

node& node::operator=(node&& d) {
	attributes = std::move(d.attributes);
	tag_name = std::move(d.tag_name);
	content_text = std::move(d.content_text);
	parent = std::move(d.parent);
	children = std::move(d.children);
	d.tag_name.clear();
	d.attributes.clear();
	d.content_text.clear();
	d.parent = nullptr;
	d.children.clear();
	return *this;
}

void node::walk(std::function<bool(node&)> handler) {
	walk(*this, handler);
}

void node::walk(node& d, std::function<bool(node&)> handler) {
	if(handler(d)) {
		for(auto& c : d.children) {
			walk(*c, handler);
		}
	}
}

node_ptr node::select(const selector s) {
	auto matched_dom = shared_from_this();
	for(auto& matcher : s) {
		if(matched_dom->children.empty()) {
			return matched_dom;
		}
		auto selectee_dom = std::move(matched_dom);
		matched_dom = std::make_shared<node>();
		for(auto& c : selectee_dom->children) {
			walk(*c, [&](node& i) {
				if(matcher(i)) {
					matched_dom->children.push_back(i.shared_from_this());
					return false;
				}
				return true;
			});
		}
	}
	return matched_dom;
}

void node::to_html(std::ostream& out, bool child, int deep, char ind, bool& is_block) const {
	std::streamoff pos = out.tellp();
	if(type_node == node_t::none) {
		bool is_block_n = false;
		for(auto& c : children) {
			c->to_html(out, child, deep, ind, is_block_n);
		}
	} else if(type_node == node_t::text) {
		if(std::any_of(content_text.begin(), content_text.end(), [](char c) {
			return !std::isspace(c);
		})) {
			auto str = content_text;
			if(parent->tag_name != "script" && parent->tag_name != "style") {
				str = std::regex_replace(str, std::regex("[\\s]+"), " ");
			}
			if(is_block) {
				out << "\n" << std::string(deep, ind);
			}
			out << str;
			is_block = false;
		}
	} else if(type_node == node_t::tag) {
		bool old_is_block = is_block;
		is_block = std::find(inline_tags.begin(), inline_tags.end(), tag_name) == inline_tags.end();
		if((old_is_block || is_block) && pos && child) {
			out << "\n" << std::string(deep, ind);
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
				bool new_is_block = false;
				for(auto& c : children) {
					c->to_html(out, child, deep + 1, ind, new_is_block);
				}
				if(new_is_block) {
					out << "\n" << std::string(deep, ind);
				}
			}
			out << "</" << tag_name << ">";
		}
	} else if(type_node == node_t::comment) {
		is_block = true;
		if(pos) {
			out << "\n" << std::string(deep, ind);
		}
		out << "<!--" << content_text << "-->";
	} else if(type_node == node_t::doctype) {
		is_block = true;
		if(pos) {
			out << "\n" << std::string(deep, ind);
		}
		out << "<!DOCTYPE" << content_text << ">";
	}
}

std::string node::to_html(char ind, bool child) const {
	std::stringstream ret;
	bool is_block = false;
	to_html(ret, child, 0, ind, is_block);
	return ret.str();
}

std::string node::get_attr(const std::string& attr) const {
	auto it = attributes.find(attr);
	if (it == attributes.end()) {
		return std::string();
	}
	return it->second;
}

void node::set_attr(const std::string& key, const std::string& val) {
	attributes[key] = val;
}

void node::append(node_ptr& n) {
	n->parent = this;
	children.push_back(n);
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
	if(new_node->type_node == node_t::tag) {
		if(new_node->type_tag == tag_t::open) {
			new_node->index = current_ptr->node_count++;
			current_ptr->children.push_back(new_node);
			if(!new_node->self_closing) {
				current_ptr = new_node.get();
				if(new_node->tag_name == "script") {
					state = STATE_SCRIPT_DATA;
				}
			}
			(*this)(*new_node);
		} else if(new_node->type_tag == tag_t::close) {
			auto _current_ptr = current_ptr;
			std::vector<node*> not_closed;
			while(_current_ptr->parent && _current_ptr->tag_name != new_node->tag_name) {
				not_closed.push_back(_current_ptr);
				_current_ptr = _current_ptr->parent;
			}
			if(_current_ptr->parent && _current_ptr->tag_name == new_node->tag_name) {
				for(auto& c : callback_err) {
					for(auto n : not_closed) {
						c(err_t::tag_not_closed, *n);
					}
				}
				if(!new_node->content_text.empty()) {
					auto text_node = std::make_shared<node>(current_ptr);
					text_node->type_node = node_t::text;
					text_node->content_text = std::move(new_node->content_text);
					new_node->content_text.clear();
					current_ptr->children.push_back(text_node);
				}
				current_ptr = _current_ptr->parent;
				(*this)(*new_node);
			}
		}
	} else if(new_node->type_node == node_t::text){
		if(!new_node->content_text.empty()) {
			current_ptr->children.push_back(new_node);
			(*this)(*new_node);
		}
	} else {
		current_ptr->children.push_back(new_node);
		(*this)(*new_node);
	}
	new_node = std::make_shared<node>(current_ptr);
	new_node->type_node = node_t::text;
}

node_ptr html::parser::parse(const std::string& html) {
	int state = STATE_DATA;
	bool eof = false;
	char c;
	bool reconsume = false;
	auto it = html.begin();
	auto getc = [&]() -> char {
		if(it != html.end()) {
			return *it++;
		}
		eof = true;
		return 0;
	};
	auto get_string = [&](std::string str) {
		for(std::string::size_type i = 0; i < str.size(); i++) {
			if(i) {
				c = getc();
			}
			if(eof || str[i] != c) {
				it -= i;
				return false;
			}
		}
		return true;
	};
	auto _parent = std::make_shared<node>();
	current_ptr = _parent.get();
	new_node = std::make_shared<node>(current_ptr);
	new_node->type_node = node_t::text;
	std::string k;
	do {
		if(!reconsume) {
			c = getc();
		}
		reconsume = false;
		if(eof) {
			break;
		}
		switch(state) {
			case STATE_DATA: // 0
				if(c == '<') {
					state = STATE_TAG_OPEN;
				} else {
					new_node->content_text += c;
				}
			break;
			case STATE_SCRIPT_DATA: // 4
				if(c == '<') {
					state = STATE_SCRIPT_DATA_LESS_THAN_SIGN;
				} else if(c == 0x00) {
					new_node->content_text += '_';
				} else {
					new_node->content_text += c;
				}
			break;
			case STATE_TAG_OPEN: // 6
				if(c == '!') {
					state = STATE_MARKUP_DEC_OPEN_STATE;
				} else if(c == '/') {
					state = STATE_END_TAG_OPEN;
				} else if(std::isalpha(c)) {
					state = STATE_TAG_NAME;
					handle_node();
					new_node->type_node = node_t::tag;
					new_node->type_tag = tag_t::open;
					reconsume = true;
				} else if(c == '?') {
					state = STATE_BOGUS_COMMENT;
					handle_node();
					new_node->type_node = node_t::comment;
					reconsume = true;
				} else {
					new_node->content_text += '<';
					reconsume = true;
					state = STATE_DATA;
				}
			break;
			case STATE_END_TAG_OPEN: // 7
				if(std::isalpha(c)) {
					state = STATE_TAG_NAME;
					handle_node();
					new_node->type_node = node_t::tag;
					new_node->type_tag = tag_t::close;
					reconsume = true;
				} else if(c == '>') {
					state = STATE_DATA;
				} else {
					state = STATE_BOGUS_COMMENT;
					handle_node();
					new_node->type_node = node_t::comment;
					reconsume = true;
				}
			break;
			case STATE_TAG_NAME: // 8
				if(c == 0x09 || c == 0x0A || c == 0x0C || c == 0x20) {
					state = STATE_BEFORE_ATTRIBUTE_NAME;
				} else if(c == '/') {
					state = STATE_SELF_CLOSING;
				} else if(c == '>') {
					state = STATE_DATA;
					handle_node();
				} else if(std::isalpha(c)) {
					new_node->tag_name += std::tolower(c);
				} else if(c == 0x00) {
					new_node->tag_name += '_';
				} else {
					new_node->tag_name += c;
				}
			break;
			case STATE_SCRIPT_DATA_LESS_THAN_SIGN: // 15
				if(c == '/') {
					state = STATE_SCRIPT_DATA_END_TAG_OPEN;
				} else {
					new_node->content_text += '<';
					reconsume = true;
					state = STATE_SCRIPT_DATA;
				}
			break;
			case STATE_SCRIPT_DATA_END_TAG_OPEN: // 16
				if(std::isalpha(c)) {
					new_node->type_node = node_t::tag;
					new_node->type_tag = tag_t::close;
					reconsume = true;
					state = STATE_SCRIPT_DATA_END_TAG_NAME;
				} else {
					new_node->content_text += '<';
					new_node->content_text += '/';
					reconsume = true;
					state = STATE_SCRIPT_DATA;
				}
			break;
			case STATE_SCRIPT_DATA_END_TAG_NAME: { // 17
				bool anything_else = true;
				if(c == 0x09 || c == 0x0A || c == 0x0C || c == 0x20) {
					if(new_node->tag_name == "script") {
						state = STATE_BEFORE_ATTRIBUTE_NAME;
						anything_else = false;
					}
				} else if(c == '/') {
					if(new_node->tag_name == "script") {
						state = STATE_SELF_CLOSING;
						anything_else = false;
					}
				} else if(c == '>') {
					if(new_node->tag_name == "script") {
						state = STATE_DATA;
						handle_node();
						anything_else = false;
					}
				} else if(std::isalpha(c)) {
					new_node->tag_name += std::tolower(c);
					anything_else = false;
				}
				if(anything_else) {
					new_node->content_text += '<';
					new_node->content_text += '/';
					new_node->content_text += new_node->tag_name;
					new_node->tag_name.clear();
					reconsume = true;
					state = STATE_SCRIPT_DATA;
				}
			}
			break;
			case STATE_BEFORE_ATTRIBUTE_NAME: // 32
				if(c == 0x09 || c == 0x0A || c == 0x0C || c == 0x20) {
					// skip
				} else if(c == '/' || c == '>') {
					reconsume = true;
					state = STATE_AFTER_ATTRIBUTE_NAME;
				} else if(c == '=') {
					k = c;
					state = STATE_ATTRIBUTE_NAME;
				} else {
					k.clear();
					reconsume = true;
					state = STATE_ATTRIBUTE_NAME;
				}
			break;
			case STATE_ATTRIBUTE_NAME: // 33
				if(c == 0x09 || c == 0x0A || c == 0x0C || c == 0x20 || c == '/' || c == '>') {
					new_node->attributes[k];
					reconsume = true;
					state = STATE_AFTER_ATTRIBUTE_NAME;
				} else if(c == '=') {
					new_node->attributes[k];
					state = STATE_BEFORE_ATTRIBUTE_VALUE;
				} else if(c == 0x00) {
					k += '_';
				} else if(c == '\'' || c == '"' || c == '<') {
					k += c;
				} else {
					k += std::tolower(c);
				}
			break;
			case STATE_AFTER_ATTRIBUTE_NAME: // 34
				if(c == 0x09 || c == 0x0A || c == 0x0C || c == 0x20) {
					// skip
				} else if(c == '/') {
					state = STATE_SELF_CLOSING;
				} else if(c == '=') {
					state = STATE_BEFORE_ATTRIBUTE_VALUE;
				} else if(c == '>') {
					state = STATE_DATA;
					handle_node();
				} else {
					k.clear();
					reconsume = true;
					state = STATE_ATTRIBUTE_NAME;
				}
			break;
			case STATE_BEFORE_ATTRIBUTE_VALUE: // 35
				if(c == 0x09 || c == 0x0A || c == 0x0C || c == 0x20) {
					// skip
				} else if(c == '"') {
					state = STATE_ATTRIBUTE_VALUE_DOUBLE;
				} else if(c == '\'') {
					state = STATE_ATTRIBUTE_VALUE_SINGLE;
				} else if(c == '>') {
					state = STATE_DATA;
					handle_node();
				} else {
					reconsume = true;
					state = STATE_ATTRIBUTE_VALUE_UNQUOTED;
				}
			break;
			case STATE_ATTRIBUTE_VALUE_DOUBLE: // 36
				if(c == '"') {
					state = STATE_AFTER_ATTRIBUTE_VALUE_QUOTED;
				} else if(c == 0x00) {
					new_node->attributes[k] += '_';
				} else {
					new_node->attributes[k] += c;
				}
			break;
			case STATE_ATTRIBUTE_VALUE_SINGLE: // 37
				if(c == '\'') {
					state = STATE_AFTER_ATTRIBUTE_VALUE_QUOTED;
				} else if(c == 0x00) {
					new_node->attributes[k] += '_';
				} else {
					new_node->attributes[k] += c;
				}
			break;
			case STATE_ATTRIBUTE_VALUE_UNQUOTED: // 38
				if(c == 0x09 || c == 0x0A || c == 0x0C || c == 0x20) {
					state = STATE_BEFORE_ATTRIBUTE_NAME;
				} else if(c == '>') {
					state = STATE_DATA;
					handle_node();
				} else if(c == 0x00) {
					new_node->attributes[k] += '_';
				} else if(c == '"' || c == '\'' || c == '<' || c == '=' || c == '`') {
					new_node->attributes[k] += c;
				} else {
					new_node->attributes[k] += c;
				}
			break;
			case STATE_AFTER_ATTRIBUTE_VALUE_QUOTED: // 39
				if(c == 0x09 || c == 0x0A || c == 0x0C || c == 0x20) {
					state = STATE_BEFORE_ATTRIBUTE_NAME;
				} else if(c == '/') {
					state = STATE_SELF_CLOSING;
				} else if(c == '>') {
					state = STATE_DATA;
					handle_node();
				} else {
					reconsume = true;
					state = STATE_BEFORE_ATTRIBUTE_NAME;
				}
			break;
			case STATE_SELF_CLOSING: // 40
				if(c == '>') {
					new_node->self_closing = true;
					state = STATE_DATA;
					handle_node();
				} else {
					reconsume = true;
					state = STATE_BEFORE_ATTRIBUTE_NAME;
				}
			break;
			case STATE_BOGUS_COMMENT: // 41
				if(c == '>') {
					state = STATE_DATA;
					handle_node();
				} else if(c == 0x00) {
					new_node->content_text += '_';
				} else {
					new_node->content_text += c;
				}
			break;
			case STATE_MARKUP_DEC_OPEN_STATE: // 42
				if(get_string("--")) {
					state = STATE_COMMENT_START;
					handle_node();
					new_node->type_node = node_t::comment;
				} else if(get_string("DOCTYPE")) {
					state = STATE_DOCTYPE;
					handle_node();
					new_node->type_node = node_t::doctype;
				} else {
					state = STATE_BOGUS_COMMENT;
					handle_node();
					new_node->type_node = node_t::comment;
					new_node->bogus_comment = true;
					new_node->content_text += c;
				}
			break;
			case STATE_COMMENT_START: // 43
				if(c == '-') {
					state = STATE_COMMENT_START_DASH;
				} else if(c == '>') {
					state = STATE_DATA;
					handle_node();
				} else {
					reconsume = true;
					state = STATE_COMMENT;
				}
			break;
			case STATE_COMMENT_START_DASH: // 44
				if(c == '-') {
					state = STATE_COMMENT_END;
				} else if(c == '>') {
					state = STATE_DATA;
					handle_node();
				} else {
					new_node->content_text += '-';
					state = STATE_COMMENT;
				}
			break;
			case STATE_COMMENT: // 45
				if(c == '-') {
					state = STATE_COMMENT_END_DASH;
				} else if(c == 0x00) {
					new_node->content_text += '_';
				} else {
					new_node->content_text += c;
				}
			break;
			case STATE_COMMENT_END_DASH: // 50
				if(c == '-') {
					state = STATE_COMMENT_END;
				} else {
					new_node->content_text += '-';
					state = STATE_COMMENT;
				}
			break;
			case STATE_COMMENT_END: // 51
				if(c == '>') {
					state = STATE_DATA;
					handle_node();
				} else if(c == '-') {
					new_node->content_text += c;
				} else {
					new_node->content_text += '-';
					new_node->content_text += '-';
					reconsume = true;
					state = STATE_COMMENT;
				}
			break;
			case STATE_DOCTYPE: // 53
				if(c == '>') {
					state = STATE_DATA;
					handle_node();
				} else if(c == 0x00) {
					new_node->content_text += '_';
				} else {
					new_node->content_text += c;
				}
			break;
		}
	} while(c || reconsume);
	new_node->type_node = node_t::text;
	handle_node();
	return _parent;
}

}