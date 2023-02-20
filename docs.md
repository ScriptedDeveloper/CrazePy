
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

# elif statements
```
var` myVar = false;
if(myVar == true) {
    print("myVar is true;\n");
} elif(myVar == false) {
    print("myVar is false\n");
}
```
Elif statements are a combination of C and Python syntax, just as promised!

# While loops
While loops work just as C loops do. 

```
var myVar = 0;
while(myVar != 5) {
    myVar = myVar + 1;
}
print(myVar);
```

# Input function
Input function works by taking an input and saving it in a variable
```
var myInput = input();
print(myInput);
```
**Warning ! Depending on what kind of data type you have (for example string), you need a newline in order for it to be outputted in zsh terminals.**


