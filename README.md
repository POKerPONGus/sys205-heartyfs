# HeartyFS

**HeartyFS** is a simple command-line interface (CLI) program that simulates a basic file system. It supports fundamental file and directory operations with limited functionality compared to GNU counterparts. HeartyFS runs on Linux, allowing some interaction with the host OS file system for convenience.

## Commands

1. **cd**  
   *Syntax*: `heartyfs cd <dir-path>`
   
   Change the current directory in the simulated file system.


2. **ls**  
   *Syntax*: `heartyfs ls [dir-path]`

   List the contents of the current directory or directory indicated.

3. **pwd**  
   *Syntax*: `heartyfs pwd`

   Print the current directory path.
   
4. **mkdir**  
   *Syntax*: `heartyfs mkdir <dir-path>`

   Create a new directory.

5. **rmdir**  
   *Syntax*: `heartyfs rmdir <dir-path>`

   Remove an empty directory.

6. **create**  
   Create a new file in the current directory.
   
   *Syntax*: `heartyfs create <file-path>`

7. **rm**  
   *Syntax*: `heartyfs rm <file-path>`

   Remove a file.

8. **write**  
   *Syntax*: `heartyfs write [options] <dest-path> [src-path]`

   Write to an existing file. Supports two modes:
   - `-w` or no option: Write mode (overwrite).
   - `-a`: Append mode.
  
   Input can be provided either from `stdin` or from another file within the simulation.

9. **read**  
   *Syntax*: `heartyfs read <file-path>`

   Display the content of a file, similar to `cat`.

**Note**: Only the `write` command supports options. All commands are implemented with minimal features compared to their GNU counterparts.

## Options

HeartyFS also includes several global options to manage the simulation environment:

- **--help**  
  Display a help message listing all the commands and options.

- **--reset**  
  Clear the entire file system and revert to its initial state.

- **--print-bitmap**  
  Display the bitmap used to track which blocks are in use.

## Examples

1. **Creating a File**:
   ```bash
   heartyfs create myfile.txt
   ```
2. **Writing to a File**:
   ```bash
   echo "Hello, world!" | heartyfs write -w myfile.txt
   heartyfs write -a myfile.txt < otherfile.txt
   ```
3. **Reading a File**
   ```
   bashheartyfs read myfile.txt
   ```