# lightweight-yaml-parser
A minimal single-header C++ library for reading and parsing .yaml files.
### Design goals
This project was designed to be as easy to use as possible. It is not the best YAML parser out there, but the goal in mind was for it to be **easy to integrate** and **easy to use** in your project. **If you are looking for basic functionality such as reading a yaml configuration file, then this project is for you.** If, however, you need advanced capabilities like generating your own YAML files, then you probably should look elsehwere (though, I might consider expanding this project at some point).
### How to install/integrate
Since this is a single-header library, all you have to do is copy ```yaml.hpp``` into your project and include it via
```cpp
#include "yaml.hpp"
```
### Examples
The library, as of right now, can only parse YAML files, so there isn't much need for a documentation. For an extensive example, please refer to ```example.cpp```. There you will be able to see most use cases be shown and explained in detail. Below is a more basic example to show you what the syntax looks like:
```cpp
#include <iostream>
#include <iomanip>
#include "yaml.hpp"

int main()
{
	yaml::YAML yaml_file("example.yaml");
	
	std::string text = yaml_file["text"].cast<std::string>();
	std::cout << text << "\n";

	float pi = yaml_file["pi"].cast<double>();
	std::cout << std::fixed << "PI = " << pi << "\n";

	if(yaml_file["linux_is_better_than_windows"].cast<bool>())
		std::cout << "W take\n";
	else
		std::cout << "L take\n";

	std::cout << "Inner object's name: " << yaml_file["outer_object"]["inner_object"]["name"].cast<std::string>() << "\n";
	
	std::cout << "Number from matrix: " << yaml_file["matrix"][1][1].cast<int>() << "\n";


	return 0;
}

```
