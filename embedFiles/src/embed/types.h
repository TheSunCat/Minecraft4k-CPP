#pragma once

struct embedded_binary
{
    const unsigned int* data;
    unsigned int size;

    constexpr embedded_binary(const unsigned int* d, unsigned int s) : data(d), size(s) {}
};


// todo(Gustav): add compressed_binary

