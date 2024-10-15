# Font Previewer 

## Introduction
The problem I want to solve with this tool, is to have a quick preview how a
text would look like with some of the installed fonts on my system. To keep it
as simple as possible, this tool just creates a very simple svg file which can
then be openend for example with Inkscape.

In my specific case, I have a folder `~/.fonts/0_for-comercial-use` where I
store all fonts which are allowed to be used for comercial purposes. So when I
now want to see how a text would look like with these fonts, I simply launch
`font_preview` with the text as an argument. 

## TODOs
- Filepath filter is currently hardcoded to `0_for-comercial-use`, this should
  be configurable via launch argument
- Configfile, Environment variables or anything else to get rid of hardcoded
  font height/width factors


