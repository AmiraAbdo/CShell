# CShell made as a project for Operating Systems course

The main function is the main entry point, which changes the color of the shell in the command line and then runs the loop function. From there, the loop function reads the line, splits it into arguments and then executes the function with the given arguments (of course, by calling the corresponding functions for each of these actions). There are a few built-in functions which can be called- mv, date, rev, du and exit (corresponding to the actions done by Linux commands of the same name).
