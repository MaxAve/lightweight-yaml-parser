#include <iostream>
#include "yaml.hpp"

int main()
{
	// Load and parse .yaml files by initializing a yaml::YAML object
	yaml::YAML yaml_file("example.yaml");
	
	std::string name = yaml_file["name"].cast<std::string>();
	int age          = yaml_file["age"].cast<int>();
	bool employed    = yaml_file["employed"].cast<bool>();

	std::cout << "name: " << name << " | age: " << age << " | employed: " << employed << "\n"; // Expected: name: John Doe | age: 69 | employed: 1
	
	yaml::Array data_array = yaml_file["data"].cast<yaml::Array>(); // You can extract arrays with either yaml::Array or std::vector<yaml::TypedValue>
 	
	// Expected: 1.0 | 6.9 | 3.1415926 |
	std::cout << "Data array:\n";
	for(int i = 0; i < yaml_file["data"].size(); i++)
	{
		std::cout << "\t" << yaml_file["data"][i].cast<double>() << "\n";
	}
	
	// Acessing values from nested arrays:
	int value_from_nested_array = yaml_file["nested_arrays"][2][1][1].cast<int>();
	std::cout << "value_from_nested_array = " << value_from_nested_array << "\n"; // Expected: 69420

	// Outputting the interpreted data type of a value:
	yaml::YAMLType type_of_name = yaml_file["name"].type;
	std::cout << "Type of name: " << yaml::yaml_type_to_str(type_of_name) << "\n";

	return 0;
}
