#pragma once

#ifndef _INDEXBUFFER_
#define _INDEXBUFFER_

#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>


class IndexBuffer
{
public:
    virtual ~IndexBuffer() = default;

    virtual void bind() const = 0;
    virtual void unbind() const = 0;

    virtual void set_data(const void* data, uint32_t count) = 0;

    virtual unsigned get_count() const = 0;


};

#endif

