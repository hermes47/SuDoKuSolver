// StringAdapter.cpp
#include "StringAdapter.h"
#include <iostream>

template<typename T>
StringAdapter<T>::StringAdapter(T* data)
    :m_data(data)
{}

template<typename T>
void StringAdapter<T>::doAdapterStuff()
{
    std::cout << std::string(m_data) << std::endl;
}

// Explicitly instantiate only the classes you want to be defined.
// In this case I only want the template to work with characters but
// I want to support both char and wchar_t with the same code.
template class StringAdapter<char>;
//template class StringAdapter<wchar_t>;
