#include "StringAdapter.h"

// Note: Main can not see the definition of the template from here (just the declaration)
//       So it relies on the explicit instantiation to make sure it links.
int main()
{
  StrAdapter  x("hi There");
  x.doAdapterStuff();
}
