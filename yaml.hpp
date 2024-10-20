// Simple YAML parser implementation
// Note: the parser currently does NOT support nested lists (e.g. [[1, 2, 3], [4, 5, 6]])

#include <iostream>
#include <unordered_map>
#include <fstream>
#include <variant>
#include <vector>
#include <array>

#define TYPE_NONE   0
#define TYPE_INT    1
#define TYPE_DOUBLE 2
#define TYPE_STRING 3
#define TYPE_ARRAY  4

typedef std::variant<int, double, std::string, std::vector<std::variant<int, double, std::string>>> value_;
typedef std::unordered_map<std::string, value_> YAML;

typedef struct
{
	std::string name;
	value_ value;
} Variable;

// Removes tabs and spaces at the beginning and end of the string
// E.g. "   name: person " -> "name: person"
std::string truncate_spaces(const std::string &str)
{
	std::string new_str = "";

	int trailing_spaces = 0;

	for(int i = str.length() - 1; i >= 0; i--)
	{
		if(new_str.length() == 0 && (str[i] == ' ' || str[i] == '\t'))
		{
			trailing_spaces++;
			continue;
		}
		break;
	}

	for(int i = 0; i < str.length() - trailing_spaces; i++)
	{
		if(new_str.length() == 0 && (str[i] == ' ' || str[i] == '\t'))
			continue;
		new_str += str[i];
	}

	return new_str;
}

// Removes all tabs and spaces that come after a specified char
// E.g. "name:    person" -> "name:person"
std::string remove_spaces_after_char(const std::string &str, char c)
{
	std::string new_str = "";
	char last_char = '\0';
	for(int i = new_str.length(); i < str.length(); i++)
	{
		if(str[i] != ' ')
			last_char = str[i];
		if(last_char == c && str[i] == ' ')
			continue;
		new_str += str[i];
	}
	return new_str;
}

// Returns true if the YAML line is a variable-value pair, otherwise returns false
bool is_variable_value_pair(const std::string &str)
{
	return (str.find(':') != (str.length() - 1));
}

// Remove a character completely from the given string
std::string filter(const std::string &str, char c)
{
	std::string new_str;
	for(int i = 0; i < str.length(); i++)
	{
		if(str[i] == c)
			continue;
		new_str += str[i];
	}
	return new_str;
}

// Splits a string into the name and value based on the : delimiter
// Example: "variable_name:value" -> {"variable_name", "value"}
std::array<std::string, 2> split_variable(const std::string &str)
{
	std::string name, value;
	int delimiter_pos = str.find(':');
	name = str.substr(0, delimiter_pos);
	value = str.substr(delimiter_pos + 1, str.length());
	std::array<std::string, 2> name_value = {name, value};
	return name_value;
}

// Get the data type of the YAML value
uint8_t get_type(const std::string &str)
{
	if(str.length() == 0)
		return TYPE_NONE;

	// Array
	if(str[0] == '[' && str[str.length() - 1] == ']')
		return TYPE_ARRAY;
	
	// Int
	bool is_int = true;
	for(int i = 0; i < str.length(); i++)
	{
		if(str[i] < '0' || str[i] > '9')
		{
			is_int = false;
			break;
		}
	}
	if(is_int)
		return TYPE_INT;

	// Double (only 1 dot may be present; otherwise will be interpreted as a string. E.g. 420.69 -> double; 127.0.0.1 -> string)
	int number_of_dots = 0;
	for(int i = 0; i < str.length(); i++)
	{
		if(str[i] == '.')
			number_of_dots++;
		if(number_of_dots > 1)
			break;
	}
	if(number_of_dots == 1)
		return TYPE_DOUBLE;
	
	// Everything else is interpreted as a String
	return TYPE_STRING;
}

// Splits string by a given delimiter (will do so for multiple delimiters)
std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
	std::string s = str;
    	std::vector<std::string> tokens;
    	size_t pos = 0;
    	std::string token;
    	while ((pos = s.find(delimiter)) != std::string::npos) {
        	token = s.substr(0, pos);
        	tokens.push_back(token);
        	s.erase(0, pos + delimiter.length());
    	}
    	tokens.push_back(s);
    	return tokens;
}

// Converts a YAML line to a Variable struct (line must not contain extra spaces or tabs; use remove_spaces_after_char and truncate_spaces beforehand)
Variable str_to_var(const std::string &str)
{
	Variable var;
	auto name_value_pair = split_variable(str);
	var.name = name_value_pair[0];
	uint8_t type = get_type(name_value_pair[1]);
	if(type == TYPE_NONE)
	{
		var.name = "";
		var.value = "";
		return var;
	}
	switch(type)
	{
	case TYPE_INT:
		var.value = std::stoi(name_value_pair[1]);
		break;
	case TYPE_DOUBLE:
		var.value = std::stod(name_value_pair[1]);
		break;
	case TYPE_STRING:
		var.value = name_value_pair[1];
		break;
	case TYPE_ARRAY:
		std::string s = filter(filter(name_value_pair[1], '['), ']'); // Remove brackets
		std::vector<std::string> values = split(s, ","); // Split values by comma
		std::vector<std::variant<int, double, std::string>> array;
		for(int i = 0; i < values.size(); i++)
		{
			std::variant<int, double, std::string> v;
			uint8_t type = get_type(values.at(i));
			switch(type)
			{
			case TYPE_INT:
				var.value = std::stoi(values.at(i));
				break;
			case TYPE_DOUBLE:
				var.value = std::stod(values.at(i));
				break;
			case TYPE_STRING:
				var.value = values.at(i);
				break;
			}
			array.push_back(v);
		}
		var.value = array;
		break;
	}
	return var;
}

bool is_valid(const Variable &var)
{
	return var.name.length() > 0;
}

// Parses the provided YAML file and returns a hashmap representing the YAML data structure
YAML parse_yaml(std::string path)
{
	YAML yaml;
	std::vector<std::string> lines;	

	std::ifstream f(path);
    	if (!f.is_open()) {
        	std::cerr << "Encountered error while trying to open " << path << "\n";
        	return yaml;
    	}
    	std::string tmp;
    	while (getline(f, tmp))
        	lines.push_back(remove_spaces_after_char(remove_spaces_after_char(truncate_spaces(tmp), ':'), ','));
    	f.close();
	
	for(int i = 0; i < lines.size(); i++)
	{
		Variable v = str_to_var(lines.at(i));
		if(is_valid(v))
			yaml[v.name] = v.value;
	}

	return yaml;
}
