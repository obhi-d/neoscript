
#include <string>
#include <string_view>
#include <memproy>
#include <cstdint>

namespace neo {

/// Parameter Layout
class command {
	struct value {

		virtual std::string_view name() const noexcept = 0;
		virtual std::string_view value() const noexcept = 0;
		virtual std::size_t count() const noexcept = 0;
		virtual value const& at(std::uint32_t i) const noexcept = 0;
		virtual value& at(std::uint32_t i) noexcept = 0;
	};
public:
	class single : value {
	public:
		void set_name(std::string_view name) override {
			name_ = name;
		}
		void set_value(std::string_view value) override {
			value_ = value;
		}

		virtual std::size_t count() const override {
			return 0;
		}
		virtual value const& at(std::uint32_t i) const override {
			return *this;
		}
		virtual value& at(std::uint32_t i) noexcept {
			return *this;
		}
		virtual std::string_view name() const noexcept {
			return name_;
		}
		virtual std::string_view value() const noexcept {
			return name_;
		}

	private:
		std::string name_;
		std::string value_;
	};

	class list : value {
	public:
		using vector = std::vector<std::unique_ptr<value>>;
		using iterator = vector::iterator;
		using const_iterator = vector::const_iterator;

		iterator begin() noexcept { return val_.begin(); }
		const_iterator begin() const noexcept { return val_.begin(); }
		iterator end() noexcept { return val_.end(); }
		const_iterator end() const noexcept { return val_.end(); }

		void set_name(std::string_view name) noexcept {
			name_ = name;
		}
		void set_value(std::string_view) noexcept {}

		void emplace_back(std::unique_ptr<value>&& i_param) noexcept {
			value_.emplace_back(std::move(i_param));
		}

		virtual std::size_t count() const noexcept {
			return value_.size();
		}
		virtual value const& at(std::uint32_t i) const noexcept {
			return *value_[i].get();
		}
		virtual std::string_view name() const {
			return name_;
		}
		virtual std::string_view value() const {
			return "";
		}
	private:
		std::string name_;
		vector value_;
	};

public:

	std::string_view name() const {
		return name_;
	}

	list const& params() const {
		return params_;
	}

	list& params() {
		return params_;
	}

private:
	std::string name_;
	list params_;
};

}