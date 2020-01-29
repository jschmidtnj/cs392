# day 2

## commands

   u   g   o
d rwx rwx rwx

d = directory
u = user
g = group
o = other

- `chmod o-w file.txt`
  - removes write access from other
- shorthand: `chmod 755 script.sh`
  - 7 = rwx, 5 = r-x, 5 = r-x
  - 644 rw-r--r--
- `ls -al` shows how many ways you can get to that folder / file
- symbolic links are shortcuts
- arrays:
  - `declare -a array`
  - `array[0]="hi"`
  - `array[0]="bye"`
  - `echo ${array[0]}`
  - `echo ${array[*]}` - prints everything
  - `echo ${#array[*]}` - count num elements in array
- `basename <filepath>` - shows the name of the file
- `echo $?` gets the return code of the previous execution
