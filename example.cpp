#include <iostream>
#include "yaml.hpp"

int main()
{
	// Load and parse .yaml files by initializing a yaml::YAML object
	yaml::YAML yaml_file("example.yaml");
	
	// You can access values with yaml_file["VARIABLE_NAME"]
	// Values are stored in yaml::YAML::data need to be cast to a data type. You have to use the yaml::get function to do that.
	// Note: if you get a segfault or a bad_alloc error with yaml::get, make sure you are using correct data types and variable names!
	std::string name = yaml::get<std::string>(yaml_file["name"]);
	int age          = yaml::get<int>(yaml_file["age"]);
	bool employed    = yaml::get<bool>(yaml_file["employed"]);

	std::cout << "name: " << name << " | age: " << age << " | employed: " << employed << "\n"; // Expected: name: John Doe | age: 69 | employed: 1
	
	yaml::Array data_array = yaml::get<yaml::Array>(yaml_file["data"]); // You can extract arrays with either yaml::Array or std::vector<yaml::TypedValue>
 	
	// Expected: 1.0 | 6.9 | 3.1415926 |
	std::cout << "Data array:\n";
	for(int i = 0; i < yaml_file["data"].size(); i++)
	{
		//std::cout << "\t" << yaml::get<double>(data_array[i]) << "\n";
		std::cout << "\t" << yaml::get<double>(yaml_file["data"][i]) << "\n";
	}
	
	// Acessing values from nested arrays:
	int value_from_nested_array = yaml::get<int>(yaml_file["nested_arrays"][2][1][1]);
	std::cout << "value_from_nested_array = " << value_from_nested_array << "\n"; // Expected: 69420

	// Outputting the interpreted data type of a value:
	yaml::YAMLType type_of_name = yaml_file["name"].type;
	std::cout << "Type of name: " << yaml::yaml_type_to_str(type_of_name) << "\n";

	return 0;
}
