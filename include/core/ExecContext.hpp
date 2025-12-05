#pragma once
#include <iostream>

struct ExecContext {
    std::istream& in;
    std::ostream& out;
    std::ostream& err;
    ExecContext(std::istream& inStream, std::ostream& outStream, std::ostream& errStream)
        : in(inStream), out(outStream), err(errStream) {}
};
