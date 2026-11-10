#include <iostream>

#include "include/unq_ptr.h"

int main() {
    UnqPtr<int> first(new int(10));

    std::cout << *first << '\n';

    UnqPtr<int> second = first;

    *second = 20;

    std::cout << *first << '\n';
    std::cout << *second << '\n';

    return 0;
}