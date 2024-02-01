#ifndef __ASSERT_H__
#define __ASSERT_H__

#include <iostream>

#define DEBUGBREAK  __debugbreak()
#define ASSERT(x,message)   if (!(x)) { std::cerr << "!! Assertion failed! " << message; DEBUGBREAK; }

#endif // __ASSERT_H__