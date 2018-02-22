#include <string>
// StringAdapter.h
template<typename T>
class StringAdapter
{
     public:
         StringAdapter(T* data);
         void doAdapterStuff();
     private:
         std::basic_string<T> m_data;
};
typedef StringAdapter<char>    StrAdapter;
typedef StringAdapter<wchar_t> WStrAdapter;
