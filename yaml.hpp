// Simple YAML parser implementation
// Note: the parser currently does NOT support nested lists (e.g. [[1, 2, 3], [4, 5, 6]])

#include <iostream>
#include <unordered_map>
#include <fstream>
#include <variant>
#include <vector>
#include <array>
#include <exception>

#define TYPE_NONE     0
#define TYPE_INT      1
#define TYPE_DOUBLE   2
#define TYPE_STRING   3
#define TYPE_ARRAY    4
#define TYPE_BOOLEAN  5
#define TYPE_YAML_OBJ 6

namespace yaml
{
enum YAMLType
{
	None_,
	Int_,
	Double_,
	String_,
	Array_,
	Bool_,
	Object_,
};

std::string yaml_type_to_str(YAMLType type)
{
	switch(type)
	{
	case YAMLType::Int_:
		return "int"; // int
	case YAMLType::Double_:
		return "double"; // double
	case YAMLType::String_:
		return "string"; // std::string
	case YAMLType::Array_:
		return "array"; // yaml::Array
	case YAMLType::Bool_:
		return "bool"; // bool
	case YAMLType::Object_:
		return "object"; // yaml::Object
	}
	return "None";
}

typedef struct
{
	std::string name;
	YAMLType type;
	void* value;
} NamedValue; // Pointer to an arbitrary type with a variable name

class TypedValue
{
public:
	YAMLType type;
	void *value;
	
	TypedValue()
	{
		type = YAMLType::None_;
		value = nullptr;
	}

	size_t size()
	{
		switch(type)
		{
		case YAMLType::Array_:
			return static_cast<std::vector<TypedValue>*>(value)->size();
		case YAMLType::Object_:
			return static_cast<std::unordered_map<std::string, TypedValue>*>(value)->size();
		}
		std::cerr << "ERROR: Unable to get length of type " << yaml_type_to_str(type) << ". This operation is only applicable to array and object.\n";
		return -1;
	}

	TypedValue operator[](size_t index)
	{
		if(type != YAMLType::Array_)
		{
			std::cerr << "ERROR: Attempting to cast TypedValue of type " << yaml_type_to_str(type) << " to array.";
			return TypedValue();
		}
		return static_cast<std::vector<TypedValue>*>(value)->at(index);
	}

	TypedValue operator[](std::string name)
	{
		if(type != YAMLType::Object_)
		{
			std::cerr << "ERROR: Attempting to cast TypedValue of type " << yaml_type_to_str(type) << " to object.";
			return TypedValue();
		}
		return static_cast<std::unordered_map<std::string, TypedValue>*>(value)->at(name);
	}
}; // Pointer to an arbitrary type with a type enum includedi

typedef std::vector<TypedValue> YAML_array;
typedef std::unordered_map<std::string, TypedValue> YAML_map;

typedef std::string String;
typedef YAML_array  Array;
typedef YAML_map    Object;



// Casts the value from a YAML map to a specified type
// Example: int data = yaml::get<int>(yaml_file.data["data"]);
template<typename T>
T get(const TypedValue& value)
{
	return *(static_cast<T*>(value.value));
}

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
YAMLType get_type(const std::string &str)
{
	if(str.length() == 0)
		return YAMLType::None_;

	// Boolean (YAML 1.2.2 only)
	if(str == std::string("true") || str == std::string("false"))
		return YAMLType::Bool_;

	// Array
	if(str[0] == '[' && str[str.length() - 1] == ']')
		return YAMLType::Array_;
	
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
		return YAMLType::Int_;

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
		return YAMLType::Double_;
	
	// Everything else is interpreted as a String
	return YAMLType::String_;
}

// Splits string by a given delimiter (will do so for multiple delimiters)
// TODO split while accounting for scope
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

NamedValue str_to_value(const std::string &str)
{
	NamedValue val;
	val.value = nullptr;
	
	std::array<std::string, 2> name_value = split_variable(str);
	YAMLType type = get_type(name_value[1]);
	
	val.type = type;
	val.name = name_value[0];
	
	if(type == YAMLType::None_)
	{
		val.name = "";
		return val;
	}

	switch(type)
	{
	case YAMLType::Int_:
		val.value = new int(std::stoi(name_value[1]));
		break;
	case YAMLType::Double_:
		val.value = new double(std::stod(name_value[1]));
		break;
	case YAMLType::String_:
		val.value = new std::string(name_value[1]);
		break;
	case YAMLType::Array_:
	{
		val.value = new Array();
		std::string values_str = name_value[1].substr(1, name_value[1].length() - 2);
		std::vector<std::string> values_split = split(values_str, ",");
		for(int i = 0; i < values_split.size(); i++)
		{
			std::string named_val = "_:" + values_split.at(i);
			std::cout << "[DEBUG] "<< named_val << "\n";
			TypedValue new_val;
			NamedValue nval = str_to_value(values_split.at(i));
			new_val.type = nval.type;
			new_val.value = nval.value;
			static_cast<Array*>(val.value)->push_back(new_val);
		}
		break;
	}
	case YAMLType::Bool_:
		val.value = new bool(name_value[1] == std::string("true"));
		break;
	case YAMLType::Object_:
		std::cout << "no implementation yet\n";
		break;
	}
	
	return val;
}

bool is_valid(const NamedValue &val)
{
	return val.value != nullptr;
}

// Parses the provided YAML file and returns a hashmap representing the YAML data structure
YAML_map parse(std::string path)
{
	YAML_map yaml;
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
		NamedValue v = str_to_value(lines.at(i));
		if(is_valid(v))
		{
			yaml[v.name].value = v.value;
			yaml[v.name].type = v.type;
		}
	}

	return yaml;
}

class YAML
{
public:
	YAML_map data;

	YAML(std::string path)
	{
		data = parse(path);
	}

	~YAML()
	{
		//TODO deallocate everything
		std::cout << "yaml go bye bye :(\n";
	}

	size_t size()
	{
		return data.size();
	}
	
	TypedValue operator[](std::string name)
	{
		return data[name];
	}
};
} // namespace yaml
