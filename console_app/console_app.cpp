
#include <crtdbg.h>
#include <new>
#include <utility>
#include <string>
#include <vector>
#include <queue>
#include <iostream>

using namespace std;

struct value_type;

struct list
{
    value_type* prev;
    value_type* next;
};

struct value_type
{
    list list;
};

int main()
{
    list value_type::* pointer = &value_type::list;
    list list;
    ( list.next->*pointer ).next;
}

