# lightweight-yaml-parser
A lightweight single-header C++ library for parsing YAML files.\
If you have to work with more complex YAML files, then you should look elsewhere. I made this thing in like a couple of days and I'm too lazy to finish it. This library is for those of you with projects that need to read in simple configuration files, but don't want to install an advanced YAML parser. If basic functionality with minimal hassle is all you want, then this library is for you.
### Setup guide
This is just a header, so all you have to do is download the file, put it somewhere in your codebase and include it where needed.\
On Linux, to fetch the file without downloading the git repo, type in the following command:
```bash
wget https://raw.githubusercontent.com/MaxAve/lightweight-yaml-parser/refs/heads/main/yaml.hpp
```
Then you can simply include the header in your C++ code:
```cpp
#include "yaml.hpp"
```
### Example
```cpp
#include <iostream>
#include "yaml.hpp"

int main()
{
  return 0;
}
```
