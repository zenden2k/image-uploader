/*!	Turns a table into a human readable string.
	\ref http://squirrel-lang.org/forums/thread/477.aspx
 */
function tprint(table, margin=4, tabwidth=4)
{
	local indent = function(n)
	{
		local ret = "";
		for(local i=0; i<n; ++i)
			ret += " ";
		return ret;
	}

	local ret = "";
	foreach(key, value in table)
	{
		ret += indent(margin);
		ret += key;
		switch (type(value))
		{
			case "table":
				ret += " = {\n";
				ret += tprint(value, margin + tabwidth, tabwidth);
				ret += indent(margin);
				ret += "}";
				break
			case "array":
				ret += " = [\n";
				ret += tprint(value, margin + tabwidth, tabwidth);
				ret += indent(margin);
				ret += "]";
				break
			case "string":
				ret += " = \"";
				ret += value;
				ret += "\"";
				break;
			default:
				ret += " = ";
				ret += value;
				break;
		}
		ret += "\n";
	}

	return ret;
}

/*!	Raise an error with message and stack trace.
	\ref http://www.lua.org/manual/5.1/manual.html#pdf-error
 */
function error(message, level=1)
{
	// In some cases calling tprint will crash the VM because of null pointer dereference
	message = message;// + "\nstack trace:\n" + tprint(getstackinfos(level));
	throw message;
}
