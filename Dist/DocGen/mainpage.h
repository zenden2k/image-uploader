/** 
@mainpage Image Uploader Scripting API
@version 1.3.2
\n 
<hr/> 
@section Contents Contents 
-# @ref Introduction 
-# @ref Example 
-# @ref Implement 
-# @ref Globals
-# @ref UploadFilters 
-# @ref ParsingHtml 
@section Introduction Introduction
<p>Image Uploader is using scripts written in <a href="http://www.squirrel-lang.org/" target="_blank">Squirrel 3</a> language.
Squirrel is a high level imperative, object-oriented programming language, designed to be a light-weight scripting language that fits in the size, memory bandwidth, and real-time requirements of applications like video games.</p>
<p>
<a href="http://www.squirrel-lang.org/doc/squirrel3.html" target="_blank">Squirrel 3.0 reference manual</a><br>
<a href="http://www.squirrel-lang.org/doc/sqstdlib3.html" target="_blank">Squirrel 3.0 Standard Libraries manual</a>

<p>Scripts should be saved in utf-8 encoding in files with <code>.nut</code> extension and placed in the Data/Scripts directory.
<hr/>

@section Example Example
@include example.nut 
<p>
You have to implement at least one function — <code>UploadFile</code>.<br>
If you want to support album listing/creating/modifying, you have to implement also <code>GetFolderList</code>, <code>CreateFolder</code>, 
<code>ModifyFolder</code>, <code>GetFolderAccessTypeList</code></i>.</p>

<code>nm</code> - global object - an instance of NetworkClient<br>
<code>ServerParams</code> - global object - an instance of ServerSettingsStruct 
<p>

@section ParsingHtml Parsing HTML with gumbo-query
@include gumbo_query.nut
*/