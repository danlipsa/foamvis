#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include <functional>

template <class E>
struct DeleteElement : public std::unary_function<E*, void>
{void operator() (E* e) {delete e;}};

#endif
