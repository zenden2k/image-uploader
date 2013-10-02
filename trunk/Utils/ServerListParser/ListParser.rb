=begin
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
=end

#  This script scans servers.xml file and generates HTML code  
#  containing list of server names preceded by their favicons
#  and saves it into file servers_html.txt in current folder
#
#  Unfortunately this script doesn't parse servers.xml as XML file.  
#  It simply scans it for server names using regular expression.

def file_put_contents( name, *contents )
  File.open( name, "w" ){ |file|
    contents.each{ |item|
      file << item
    }
  }
end

dataFolder =  '../../Data/'
filename= 'servers.xml'
lines = File.readlines(dataFolder  + filename)
line_count = lines.size
text = lines.join
serverNames = text.scan(/Name=\"(.+?)\"/)
serverNames.sort! { |a,b| a[0].downcase <=> b[0].downcase }

resultText = "<table>\n";
serverNames.each do |nam|
	servName = nam[0]
	resultText<< "<tr><td>";
	if( File::exists?( dataFolder + "Favicons/" + servName + ".ico" ) )
		resultText<< "<img src='images/servericons/"+ servName.downcase + ".ico' align='middle' width='16' height='16'>&nbsp;&nbsp;&nbsp;&nbsp;";
		end;
	resultText<< "</td><td>";
		resultText << servName << "</td></tr>\n";
end
resultText << "</table>"
file_put_contents("servers_html.txt", resultText)