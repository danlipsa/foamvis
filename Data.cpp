#include <functional>
#include <algorithm>
using namespace std;

#include "Data.h"

struct deleteBody : public unary_function<Body*, void>
{
    void operator() (Body* b) {delete b;}
};

Data::~Data()
{
    for_each(m_bodies.begin(), m_bodies.end(), deleteBody());
}
