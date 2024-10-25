#include <iostream>
#include "yaml.hpp"

int main()
{
	yaml::YAML yaml_file("example.yaml");
	
	std::string name = yaml_file["name"].cast<std::string>();
	int age          = yaml_file["age"].cast<int>();
	bool employed    = yaml_file["employed"].cast<bool>();

	std::cout << "name: " << name << " | age: " << age << " | employed: " << employed << "\n";
	
	yaml::Array data_array = yaml_file["data"].cast<yaml::Array>();  	
	std::cout << "Data array:\n";
	for(int i = 0; i < yaml_file["data"].size(); i++)
	{
		std::cout << "\t" << yaml_file["data"][i].cast<double>() << "\n";
	}
	
	int value_from_nested_array = yaml_file["nested_arrays"][2][1][1].cast<int>();
	std::cout << "value_from_nested_array = " << value_from_nested_array << "\n";

	yaml::YAMLType type_of_name = yaml_file["name"].type;
	std::cout << "Type of name: " << yaml::yaml_type_to_str(type_of_name) << "\n";

	for(auto& it : yaml_file.data)
	{
		std::cout << it.first << "\n";
	}

	std::cout << "COMPANY: " << yaml_file["career"]["job"]["company"]["site"].cast<std::string>() << "\nFAV NUMBER: " << yaml_file["career"]["favorite_number"].cast<double>() << "\nHEIGHT: " << yaml_file["height"].cast<double>() << "\n";

	std::cout << yaml_file["career"]["job"]["company"]["random_data"][1].cast<std::string>() << "\n";

	std::cout << yaml_file["career"]["matrix"][1][2].cast<int>() << "\n";

	std::cout << yaml_file["career"]["job"]["company"]["net_worth"].cast<int>() << "\n";

	return 0;
}
