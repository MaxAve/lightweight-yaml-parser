// Example code for parsing example.yaml

#include <iostream>
#include <iomanip>
#include "yaml.hpp"

int main()
{
	// Here is a short explanation of how data is represented:
	// Essentially, since YAML does not have static types, we use void pointers to represent data.
	// For this we have the yaml::TypedValue class which contains a pointer to the value, and an enum that represents its data type.
	// Everything, including ints, bools, doubles, strings, arrays, and objects are represented with yaml::TypedValue.
	// You can look into yaml.hpp for more info.
	// Below is a small "tutorial" for using the library:

	// Read a yaml file by initializing a YAML object with the path of the yaml file
	yaml::YAML yaml_file("example.yaml");
	
	// You can get values with square brackets (similar to JSON or python dicts)
	// In order to read in these values, you have to use the cast method to cast them to a C++ data type
	// Casting to the wrong type will usually raise a yaml::TypeMismatchException
	std::string text = yaml_file["text"].cast<std::string>(); 
	std::cout << text << "\n";

	float pi = yaml_file["pi"].cast<double>(); // Note: casting to a float will lead to an error since the parser automatically reads all floating-point values to a double
	std::cout << std::fixed << "PI = " << pi << "\n";

	if(yaml_file["linux_is_better_than_windows"].cast<bool>())
		std::cout << "W take\n";
	else
		std::cout << "L take\n";
	
	// You can put square brackets next to each other like with JSON or python dicts to access values inside of nested objects
	std::cout << "Inner object's name: " << yaml_file["outer_object"]["inner_object"]["name"].cast<std::string>() << "\n";
	
	std::cout << "Outer object's name: " << yaml_file["outer_object"]["name"].cast<std::string>() << "\n";
	
	// You can access values from arrays via indexes
	std::cout << "21st letter of the alphabet: " << yaml_file["alphabet"][20].cast<std::string>() << "\n";
	
	// The parser also supports multi-dimensional arrays
	std::cout << "Number from matrix: " << yaml_file["matrix"][1][1].cast<int>() << "\n";

	// Arrays are actually stored as vectors, so you can work with them just like you would with regular C++ vectors
	// The code blow showcases how you can loop through a 2D-array with for loops
	for(int i = 0; i < yaml_file["matrix"].size(); i++)
	{
		for(int j = 0; j < yaml_file["matrix"][i].size(); j++)
		{
			std::cout << yaml_file["matrix"][i][j].cast<int>() << " "; // ALWAYS remember to put .cast when you are trying to read the value! I myself have gotten enough unexpected errors after forgetting to use this method!
		}
		std::cout << "\n";
	}
	
	// YAML Objects are represented with hashmaps (std::unordered_map). You can loop through them as you would with a regular std::unordered_map in C++:
	std::cout << "Outer objects in outer_object:\n";
	for(auto it : yaml_file["outer_object"].cast<yaml::Object>())
	{
		std::cout << "\t" << it.first << "\n";
	}

	// If you want to loop through the YAML file itself, you have to loop through the data member, not the object itself
	std::cout << "Outer objects in the YAML file:\n";
	for(auto it : yaml_file.data)
	{
		std::cout << "\t" << it.first << "\n";
	}

	// You can print the data type of a value with the type member and the yaml::yaml_type_to_str function:
	std::cout << "Value 'matrix' has the type: " << yaml::yaml_type_to_str(yaml_file["matrix"].type) << "\n";

	return 0;
}
