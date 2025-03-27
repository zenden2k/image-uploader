/**
@file
@section Script Internationalization
*/

/**
Translates a string to the current language.

@param key represents an id of this message, should be like "scriptname.some_key1.some_key2". scriptname should be the same as your script file (without .nut extension).

@param englishText deprecated since 1.4.2
Translated strings should be stored in json file Scripts/Lang/<language_name>.json. 

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



