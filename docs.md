
# Documentation

You want to learn CrazePy and possibly contribute to the project in the future? Great! Picking up CrazePy shouldn't take you no longer than 10 minutes.

# Hello World
```
print("Hello World!\n");
```
This is as simple as it gets. Please note CrazePy requires a semicolon at the end of every expression. Please note a newline (the \n) is good practice because some terminals (like zsh) wouldn't display the output otherwise.

# Variables
```
var myVar = true;
print(myVar);
```
In CrazePy, we use the ```var``` keyword to declare a variable. Due to the language being dynamically typed, we do not need any type to specify like in C/++. Following data types are currently supported : ```bool```, ``int``, ```float```, ```string```, ```double```, ```float```.

# If statements
Let's expand our example.
```
var myVar = false;
if(myVar == false) {
    print("myVar is false.\n");
}
```
As you see, the if statement works basically like any other languages does. 
