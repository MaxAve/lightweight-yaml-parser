#include <iostream>
#include "yaml.hpp"

int main()
{
	// Load and parse .yaml files by initializing a yaml::YAML object
	yaml::YAML yaml_file("example.yaml");
	
	// You can access values with yaml_file.data["VARIABLE_NAME"].value
	// Values are stored in yaml::YAML::data need to be cast to a data type. You have to use the yaml::get function to do that.
	// Note: if you get a segfault or a bad_alloc error with yaml::get, make sure you are using correct data types and variable names!
	//       You can also check the data type by printing out yaml::yaml_type_to_str(yaml_file.data["VARIABLE_NAME"].type)
	std::string name = yaml::get<std::string>(yaml_file.data["name"].value);
	int age          = yaml::get<int>(yaml_file.data["age"].value);
	bool employed    = yaml::get<bool>(yaml_file.data["employed"].value);

	std::cout << "name: " << name << " | age: " << age << " | employed: " << employed << "\n"; // Expected: name: John Doe | age: 69 | employed: 1

	// Output the interpreted type of a value (can be used for debugging).
	std::cout << "Type of name: " << yaml::yaml_type_to_str(yaml_file.data["name"].type) << "\n";

	return 0;
}
