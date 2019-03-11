/**
@file
@section Internalization Translating scripts into different languages
*/

/**
Translates a string to the current language.

<b>key</b> represents an id of this message, should be like "scriptname.some_key1.some_key2". 
You should use string literal only for both arguments! Do not form them dynamically.
scriptname should be the same as your script file (without .nut extension).

Translated strings should be stored in json file Scripts/Lang/<language_name>.json. 
<b>language_name</b> should be the same as in program's translation filename Lang\<language_name>.lng

Aliases for this function: Translate() (you can override these functions in your code).
@since 1.3.1
*/
std::string tr(std::string key, std::string englishText);

/**
@section Example
Scripts/example.nut
@code
print(tr("example.hello_world", "Hello world!")); 
@endcode

Scripts/Lang/Spanish.json

@code
{
  "example" : {
     "hello_world" : "¡Hola, mundo!"
  }
} 
@endcode

Output:
@code
¡Hola, mundo!
@endcode
*/



