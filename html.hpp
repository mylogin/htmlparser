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
		std::vector<node_ptr>::const_iterator begin() const {
			return children.begin();
		}
		std::vector<node_ptr>::const_iterator end() const {
			return children.end();
		}
		std::vector<node_ptr>::const_iterator cbegin() const {
			return children.cbegin();
		}
		std::vector<node_ptr>::const_iterator cend() const {
			return children.cend();
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
		selector(const std::string&);
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
		enum class state_t {
			route, tag, st_class, id, st_operator, index, attr, attr_operator, attr_val
		};
		bool is_state_route(char c) {
			return c == 0 || c == ' ' || c == '[' || c == ':' || c == '.' || c == '#' || c == ',' || c == '>';
		}
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
		node* current_ptr = nullptr;
		node_ptr new_node;
		std::vector<std::pair<selector, std::function<void(node&)>>> callback_node;
		std::vector<std::function<void(err_t, node&)>> callback_err;
		enum class state_t {
			data, rawtext, tag_open, end_tag_open, tag_name, rawtext_less_than_sign, rawtext_end_tag_open, rawtext_end_tag_name, 
			before_attribute_name, attribute_name, after_attribute_name, before_attribute_value, attribute_value_double, 
			attribute_value_single, attribute_value_unquoted, after_attribute_value_quoted, self_closing, bogus_comment, 
			markup_dec_open_state, comment_start, comment_start_dash, comment, comment_end_dash, comment_end, 
			before_doctype_name, doctype_name
		} state;
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
		inline bool is_uppercase_alpha(char c) {
			return 'A' <= c && c <= 'Z';
		}
		inline bool is_lowercase_alpha(char c) {
			return 'a' <= c && c <= 'z';
		}
		inline bool is_alpha(char c) {
			return is_uppercase_alpha(c) || is_lowercase_alpha(c);
		}
		inline bool is_digit(char c) {
			return '0' <= c && c <= '9';
		}
		inline bool is_space(char c) {
			return c == 0x09 || c == 0x0A || c == 0x0C || c == 0x20 || c == 0x0D;
		}

	}

}

#endif