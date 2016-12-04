
#ifndef COMMAND_INCLUDED
#define COMMAND_INCLUDED

#include <queue>
#include <memory>
#include <utility>
#include <type_traits>

template<typename T>
int priority(const T& t) { return 0; }

template<typename T>
void execute(T& t) {}

class command_object_t
{
public:
	template<typename T>
	command_object_t(T t) : self_(std::make_shared<model_t<T>>(std::move(t))) {}

	friend int priority(const command_object_t& o) { return o.self_->priority_(); }
	friend void execute(command_object_t& o) { return o.self_->execute_(); }

private:
	struct concept_t
	{
		virtual ~concept_t() {}

		virtual int priority_() const = 0;
		virtual void execute_() = 0;
	};

	template<typename T>
	struct model_t : concept_t
	{
		model_t(T t) : data_(std::move(t)) {}

		int priority_() const
		{
			bool test = std::is_member_pointer<decltype(&T::priority)>::value;
			return test? data_.priority() : priority(data_);
		}

		void execute_()
		{
			bool test = std::is_member_pointer<decltype(&T::execute)>::value;
			test? data_.execute() : execute(data_);
		}

		T data_;
	};

	std::shared_ptr<concept_t> self_;
};

inline bool operator<(const command_object_t& lhs, const command_object_t& rhs)
{
	return priority(lhs) < priority(rhs);
}

class command_queue_t
{
public:
	bool empty() const { return queue_.empty(); }
	std::size_t size() const { return queue_.size(); }

	const command_object_t& top() const { return queue_.top(); }

	void push(const command_object_t& o) { queue_.push(o); }
	void push(command_object_t&& o) { queue_.push(o); }

	void pop() { queue_.pop(); }

private:
	std::priority_queue<command_object_t> queue_;
};

#endif /* COMMAND_INCLUDED */
