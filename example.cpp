#include <iostream>
#include "yaml.hpp"

int main()
{
	// Load and parse .yaml files by initializing a yaml::YAML object
	yaml::YAML yaml_file("example.yaml");
	
	// You can access values with yaml_file.data["VARIABLE_NAME"]
	// Values are stored in yaml::YAML::data need to be cast to a data type. You have to use the yaml::get function to do that.
	// Note: if you get a segfault or a bad_alloc error with yaml::get, make sure you are using correct data types and variable names!
	std::string name = yaml::get<std::string>(yaml_file.data["name"]);
	int age          = yaml::get<int>(yaml_file.data["age"]);
	bool employed    = yaml::get<bool>(yaml_file.data["employed"]);

	std::cout << "name: " << name << " | age: " << age << " | employed: " << employed << "\n"; // Expected: name: John Doe | age: 69 | employed: 1

	// After you're done using it, it's best to "close" the YAML object in order to deallocate the data
	yaml_file.close();

	return 0;
}
