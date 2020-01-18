# Manhunt.AudioTool
A tool to edit SFX.RAW/SDT from Manhunt (PC).


# Usage

`mhsfx <params> <input>`

# Params

### -e
Extracts input file

### -c 
Creates file with input name

### -t
Specifies Sound Data Table (.SDT) file for use with extraction or specifies name for creation

### -l
Specifies List (.LST) file for use with extraction to have nice filenames

### -r 
Specifies rebuild file with filenames and loop points required to rebuild the archive



## Example:

`mhsfx -e -t sfx.sdt -r rebuild.txt -l sfx.lst sfx.raw`

 Extracts sfx.raw with names from .lst
 
 `mhsfx -c -t new.sdt -r rebuild.txt  new.raw`

 Creates new.sdt and new.raw using rebuild file rebuild.txt.
