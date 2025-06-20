/** 
@mainpage Image Uploader Scripting API
@version 1.4.3
\n 
<hr/> 
@section Contents Contents 
-# @ref Introduction
-# @ref Implement  
-# @ref Example 
-# @ref OAuth
-# @ref Globals
-# @ref UploadFilters 
-# @ref ParsingHtml 
-# @ref Internalization 
@section Introduction Introduction
<p>Image Uploader is using scripts written in <a href="http://www.squirrel-lang.org/" target="_blank">Squirrel 3</a> language.
Squirrel is a high level imperative, object-oriented programming language, designed to be a light-weight scripting language that fits in the size, memory bandwidth, and real-time requirements of applications like video games.</p>
<p>
<a href="http://www.squirrel-lang.org/doc/squirrel3.html" target="_blank">Squirrel 3.0 reference manual</a><br>
<a href="http://www.squirrel-lang.org/doc/sqstdlib3.pdf" target="_blank">Squirrel 3.0 Standard Libraries manual (PDF)</a>

<p>Scripts should be saved in utf-8 encoding in files with <code>.nut</code> extension and placed in the Data/Scripts directory.
<hr/>

@section Example Example
@include example.nut 
<p>
You have to implement at least one function — \ref UploadFile.<br>
If you want to support album listing/creating/modifying, you have to implement also \ref GetFolderList, \ref CreateFolder, 
\ref ModifyFolder, \ref GetFolderAccessTypeList.</p>

<code>nm</code> - global object - is an instance of NetworkClient<br>
<code>ServerParams</code> - global object - is an instance of ServerSettingsStruct 
<p>

@section ParsingHtml Parsing HTML with gumbo-query
Gumbo-query is a library that provides CSS selector-like queries for Gumbo-Parser.
Use \ref ScriptAPI::Document class.
@include gumbo_query.nut
*/