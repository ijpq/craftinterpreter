#include <vector>

#include "loxvalue.h"
#include <ctime>
namespace interpreter {
class Interpreter;
}

struct LoxCallable {
	virtual size_t arity() const = 0;
	virtual LoxValueType call(interpreter::Interpreter* interpreter, 
		const std::vector<LoxValueType>& args) = 0;
	virtual std::string toString() = 0;
};

struct ClockCallable : LoxCallable {
	size_t arity() const override {return 0;}

	LoxValueType call(interpreter::Interpreter* interpreter, 
		const std::vector<LoxValueType>& args) override {
		return static_cast<double>(clock())/  CLOCKS_PER_SEC; 
	}

	std::string toString() override {return "<native fn>";}


};
struct Function : LoxCallable{
  LoxValueType call(interpreter::Interpreter* interpter,
                    const std::vector<LoxValueType>& args) override {
    // *this();
  }

  size_t arity () const override {

  }

  // template<typename ...T>
  // LoxValueType operator()(T&& ...t) {

  // }
};
