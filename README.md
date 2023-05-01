# neoscript : A Command Driven DSL 

NS relies on being parsed by a SAX parser, and drives
a command based interface while parsing files.

## Command

A basic command syntax is as follows:

```
command parameter1 parameter2 [list_parameter_1, list_parameter_2];

```

The `command` can be bound to a free function that is invoked by the engine when
encountered in the file, through the neo_registry interface.

Commands can also include an optional '=' or ':' after its name for syntatic sugar:

```
command : invoking command; // two parameters are passed to command
command = set value to command; // space is used to seperate parameters
```

String parameter value can be provided in double quoted string. 

```
command : "invoking command"; // a single parameter is passed to command
```

Lists are created using  `[]` operator

```
command : [list of parameters] // command is passed a single parameter which is a list of 3 parameters
```

Named parameters can be provided as follows:

```
command (my_parameter_name = "my_parameter value"); // parameter is named 'my_parameter_name'
```

Commands can be scoped inside other commands using `{}` scope operator

```
top_level_command 
{
	internal_command
	{

	}
}
```

## Regions

A script can contain multiple regions. Registry will callback on entering new regions.
There are two kinds of regions, text and code.

Code regions are processed by the parser, and commands are only invoked inside code regions.
Text content in text regions are directly passed to callbacks as is.

Regions are created by region seperator `--` operator at the begining of the line:

```
-- code : region --
```

A region name has two parts sepeareted by `:`. The first part is the region type. A code region must have a type `code`. 
The second part is the region id. 
For text regions, region can be of any type other than `code`.
A region id may contain spaces, but the trailing and preceding spaces are removed.

## Templates

A script might also include templates.
Templates are command group definition which are invoked based on parameters passed later in the script.

Templates are defined with the keyword `template` as follows:

```
// parameterless template
template template_name_1<> 
command ; // called without any parameters

// template with parameters
template template_name_2<parameter_A, parameter_B> 
command parameter_A parameter_B
{
	new_command parameter_B;
}

```

`template_name_1` and `template_name_2` are template names.
The command `command` will not be invoked when this definition is encountered. It can be invoked later in the file using the `using` keyword.

```
using template_name_1<>;
using template_name_2<"value 1", "value 2">;
```

## Script

A complete script might look like:

```

-- code: code section --

template shader_definition<shader_type, file>
shader shader_type
{
	files "common.glsl" "functions.glsl" "ubos.glsl" file (optional=[enable_debugging, no_optimization]);
	type shader_type;
}

using shader_definition<vertex, "vertex.glsl">;
using shader_definition<fragment, "fragment.glsl">;

template compiler<shader, options>
compile 
{
	shader_options options;
	compile shader;
}

using compiler<vertex_shader,"">;
using compiler<fragment_shader,"">;

// shaders are now compiled !

```

