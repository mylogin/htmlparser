#ifndef HTML_H
#define HTML_H

#include <memory>
#include <functional>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <cctype>
#include <algorithm>
#include <map>
#include <utility>
#include <iterator>

#define STATE_DATA 0
#define STATE_RAWTEXT 3
#define STATE_TAG_OPEN 6
#define STATE_END_TAG_OPEN 7
#define STATE_TAG_NAME 8
#define STATE_RAWTEXT_LESS_THAN_SIGN 12
#define STATE_RAWTEXT_END_TAG_OPEN 13
#define STATE_RAWTEXT_END_TAG_NAME 14
#define STATE_BEFORE_ATTRIBUTE_NAME 32
#define STATE_ATTRIBUTE_NAME 33
#define STATE_AFTER_ATTRIBUTE_NAME 34
#define STATE_BEFORE_ATTRIBUTE_VALUE 35
#define STATE_ATTRIBUTE_VALUE_DOUBLE 36
#define STATE_ATTRIBUTE_VALUE_SINGLE 37
#define STATE_ATTRIBUTE_VALUE_UNQUOTED 38
#define STATE_AFTER_ATTRIBUTE_VALUE_QUOTED 39
#define STATE_SELF_CLOSING 40
#define STATE_BOGUS_COMMENT 41
#define STATE_MARKUP_DEC_OPEN_STATE 42
#define STATE_COMMENT_START 43
#define STATE_COMMENT_START_DASH 44
#define STATE_COMMENT 45
#define STATE_COMMENT_END_DASH 50
#define STATE_COMMENT_END 51
#define STATE_BEFORE_DOCTYPE_NAME 54
#define STATE_DOCTYPE_NAME 55

#define SEL_STATE_ROUTE 0
#define SEL_STATE_TAG 1
#define SEL_STATE_CLASS 2
#define SEL_STATE_ID 3
#define SEL_STATE_OPERATOR 4
#define SEL_STATE_INDEX 5
#define SEL_STATE_ATTR 6
#define SEL_STATE_ATTR_OPERATOR 7
#define SEL_STATE_ATTR_VAL 8

#define IS_UPPERCASE_ALPHA(c) ('A' <= c && c <= 'Z')
#define IS_LOWERCASE_ALPHA(c) ('a' <= c && c <= 'z')
#define IS_ALPHA(c) (IS_UPPERCASE_ALPHA(c) || IS_LOWERCASE_ALPHA(c))
#define IS_DIGIT(c) ('0' <= c && c <= '9')
#define IS_SPACE(c) (c == 0x09 || c == 0x0A || c == 0x0C || c == 0x20 || c == 0x0D)
#define IS_STATE_ROUTE(c) (c == 0 || c == ' ' || c == '[' || c == ':' || c == '.' || c == '#' || c == ',' || c == '>')

namespace html {

	class selector;
	class parser;
	class node;

	using node_ptr = std::unique_ptr<node>;

	enum class node_t {
		none,
		text,
		tag,
		comment,
		doctype
	};

	enum class tag_t {
		none,
		open,
		close,
	};

	enum class err_t {
		tag_not_closed
	};

	class node {
	public:
		node(node* parent = nullptr) : parent(parent) {}
		node(const node&);
		node(node&& d) noexcept
		: type_node(d.type_node)
		, type_tag(d.type_tag)
		, self_closing(d.self_closing)
		, tag_name(std::move(d.tag_name))
		, content(std::move(d.content))
		, parent(nullptr)
		, bogus_comment(d.bogus_comment)
		, children(std::move(d.children))
		, attributes(std::move(d.attributes))
		, index(0)
		, node_count(d.node_count) {}
		node* at(size_t i) const {
			if(i < children.size()) {
				return children[i].get();
			}
			return nullptr;
		}
		size_t size() const {
			return children.size();
		}
		bool empty() const {
			return children.empty();
		}
		std::vector<node_ptr>::iterator begin() {
			return children.begin();
		}
		std::vector<node_ptr>::iterator end() {
			return children.end();
		}
		std::vector<node*> select(const selector, bool nested = true);
		std::string to_html(char indent = '	', bool child = true, bool text = true) const;
		std::string to_raw_html(bool child = true, bool text = true) const;
		std::string to_text(bool raw = false) const;
		node* get_parent() const {
			return parent;
		}
		bool has_attr(const std::string&) const;
		std::string get_attr(const std::string&) const;
		void set_attr(const std::string&, const std::string&);
		void set_attr(const std::map<std::string, std::string>& attributes);
		void del_attr(const std::string&);
		node& append(const node&);
		void walk(std::function<bool(node&)>);
		node_t type_node = node_t::none;
		tag_t type_tag = tag_t::none;
		bool self_closing = false;
		std::string tag_name;
		std::string content;
	private:
		node* parent = nullptr;
		bool bogus_comment = false;
		std::vector<node_ptr> children;
		std::map<std::string, std::string> attributes;
		int index = 0;
		int node_count = 0;
		void copy(const node*, node*);
		void walk(node&, std::function<bool(node&)>);
		void to_html(std::ostream&, bool, bool, int, int&, char, bool&, bool&) const;
		void to_raw_html(std::ostream&, bool, bool) const;
		void to_text(std::ostream&, bool&) const;
		friend class selector;
		friend class parser;
	};

	class selector {
	public:
		selector() = default;
		selector(const std::string);
		selector(const char* s) : selector(std::string(s)) {}
		operator bool() const {
			return !matchers.empty();
		}
	private:
		struct condition {
			condition() = default;
			condition(const condition& d) = default;
			condition(condition&&) noexcept;
			std::string tag_name;
			std::string id;
			std::string class_name;
			std::string index = "0";
			std::string attr;
			std::string attr_value;
			std::string attr_operator;
			bool operator()(const node&) const;
		};
		struct selector_matcher {
			selector_matcher() = default;
			selector_matcher(const selector_matcher&) = default;
			selector_matcher(selector_matcher&&) noexcept;
			bool operator()(const node&) const;
			bool dc_first = false;
			bool dc_second = false;
		private:
			bool all_match = false;
			std::vector<std::vector<condition>> conditions;
			friend class selector;
		};
		std::vector<selector_matcher>::const_iterator begin() const {
			return matchers.begin();
		}
		std::vector<selector_matcher>::const_iterator end() const {
			return matchers.end();
		}
		std::vector<selector_matcher> matchers;
		friend class node;
		friend class parser;
	};

	class parser {
	public:
		parser& set_callback(std::function<void(node&)> cb);
		parser& set_callback(const selector, std::function<void(node&)> cb);
		parser& set_callback(std::function<void(err_t, node&)> cb);
		void clear_callbacks();
		node_ptr parse(const std::string&);
		node_ptr parse(std::istream&);
		template<class InputIt>
		node_ptr parse(InputIt, InputIt);
	private:
		void operator()(node&);
		void handle_node();
		int state = STATE_DATA;
		node* current_ptr = nullptr;
		node_ptr new_node;
		std::vector<std::pair<selector, std::function<void(node&)>>> callback_node;
		std::vector<std::function<void(err_t, node&)>> callback_err;
	};

	namespace utils {

		node make_node(node_t, const std::string&, const std::map<std::string, std::string>& attributes = {});
		template <class T, class... Args>
		typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type
			make_unique(Args &&...args) {
			return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
		}
		template <class T>
		typename std::enable_if<std::is_array<T>::value, std::unique_ptr<T>>::type
			make_unique(std::size_t n) {
			typedef typename std::remove_extent<T>::type RT;
			return std::unique_ptr<T>(new RT[n]);

		}
		bool contains_word(const std::string&, const std::string&);
		template<class InputIt>
		bool ilook_ahead(InputIt, InputIt, const std::string&);
		std::string replace_any_copy(const std::string&, const std::string&, const std::string&);

	}

}

#endif