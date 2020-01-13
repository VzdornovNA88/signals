#include <iostream>
#include "function.hpp"
#include <functional>

#include "signal.hpp"

using namespace signals;

struct A
{
  ~A() {

    std::cout << "~A()" << std::endl;
  }

  A() {

    std::cout << "A()" << std::endl;
  }

  void foo(int a)
  {
    std::cout << "method got: " << a << std::endl;
  }

  void foo1(int a) const
  {
    std::cout << "method got const: " << a << std::endl;
    func(a+1);
  }

  function<void (int)> func = function<void (int)>::bind<A, &A::foo>(this);
};

void foo(int a)
{
  std::cout << "function got: " << a << std::endl;
}

struct B {

  void operator()(int a) {
    std::cout << "B::operator(): " << a << std::endl;
  }
};

::signal<int> signal_int2;

int main(int argc, char* argv[])
{
  auto d1(function<void (int)>::bind<foo>());

  A a;
  auto d2(function<void (int)>::bind<A, &A::foo>(&a));
  auto d3(function<void (int)>{foo});
  auto d4(function<void (int)>(&a, &A::foo));

  d1(1);
  d2(2);
  d3(3);
  d4(4);

{
  int b(2);

  auto dx(function<void ()>(
    [&,b](){std::cout << "hello world: " << b << std::endl;}));

  dx();


std::cout << "size dx: " << sizeof(dx) << std::endl;

std::cout << "1" << std::endl;

dx = []{};

std::cout << "2" << std::endl;

 dx = [a]()mutable{a.foo(777);};

std::cout << "3" << std::endl;

dx();

std::cout << "4" << std::endl;

auto  dx_copy = dx;

std::cout << "5" << std::endl;

dx_copy();
dx_copy = 0;

if( dx_copy == nullptr ) 
  std::cout << "6" << std::endl;
}

std::cout << "7" << std::endl;


  signal<std::string, int> signal;

  // attach a slot
  signal.connect([](std::string arg1, int arg2)mutable {
      std::cout << arg1 << " " << arg2 << std::endl;
  });

  signal("The answer:", 42);


  ::signal<int> signal_int;

  signal_int.connect( &a, &A::foo );

  signal_int(333);


  ::signal<int> signal_int1;

  signal_int1.connect( &a, &A::foo1 );

  signal_int1(333);

std::cout << "connect" << std::endl;
  signal_int2.connect( foo );
std::cout << "call" << std::endl;
  signal_int2(333);
std::cout << "end" << std::endl;
  ::signal<int> signal_dummy;

  ::signal<int> signal_to_signal,signal_to_signal1,signal_to_signal2;

  
  B object_b;
  signal_to_signal.connect( signal_int1 );
  signal_to_signal1.connect( signal_to_signal );
  auto connection = signal_to_signal2.connect( signal_to_signal1 );
  signal_to_signal2(1111111);

  signal_to_signal2.disconnect( connection );
  
  // std::cout << "start copy signal" << std::endl;
  // signal_int1 = signal_to_signal2;
  // std::cout << "end copy signal" << std::endl;

  std::function< void (int) > std_func = foo;


  std::cout << "d3 = " << sizeof( d3 ) << std::endl;
  std::cout << "std_func = " << sizeof( std_func ) << std::endl;
  std::cout << "sizeof( signal_to_signal2 ) = " << sizeof( signal_to_signal2 ) << std::endl;
  std::cout << "sizeof( signal_dummy ) = " << sizeof( signal_dummy ) << std::endl;
  std::cout << "sizeof( signal_int ) = " << sizeof( signal_int ) << std::endl;
  std::cout << "sizeof( signal_int1 ) = " << sizeof( signal_int1 ) << std::endl;
  std::cout << "sizeof( signal_int2 ) = " << sizeof( signal_int2 ) << std::endl;




// function<void (int)> func_object(signal_int2);
// func_object(45454);

  return 0;
}