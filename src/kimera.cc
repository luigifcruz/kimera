#include "kimera/kimera.hpp"

bool CHECK(Interfaces value, Interfaces flag) {
    return (flag & value) == flag;
}

bool CHECK(Mode value, Mode flag) {
    return (flag & value) == flag;
}