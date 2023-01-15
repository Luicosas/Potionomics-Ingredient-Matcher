# Potionomics-Ingredient-Matcher
A Potionomics ingredient matcher for getting the pesky 3 star perfect potions

![Example recipes](https://github.com/Luicosas/Potionomics-Ingredient-Matcher/blob/main/example_recipes.png)

# Intro
This is a ingredient matching program to find the highest magnium count potions possible: 

Potionomics has ingredients with different A B C D E values. We are supposed to mix the ingredients in a ration of ex. 1 1 0 0 0 for a health potion. This program takes in the ingredients list and desired potion, cauldron magnium limit, and cauldron ingredients limit then uses dynamic programming to calculate a recipe

Note: this program cannot calculate recipes with more than 30 ingredients as i am using int with 32 bits in the dp to speed it up, it shouldn't matter as the official game cauldrons only go up to 14 ingredients max
Note: The program is written in C++ with the only the standard template libraries to hopefully make it easier to read. 

# Explanation on the program

The program creates an array of size (magnium limit A + 1) * (magnium limit B + 1) * (magnium limit C + 1) * (magnium limit D + 1) * (magnium limit E + 1). 

Each index in the array correspondes to an combination of (A, B, C, D, E) and stores in the bits whether its possible to achieve this combination with 0, 1, 2, ... 30 ingredients. 

For each ingredient the program loops through the array and tries using it, i.e. dp[i + ingredient_value] |= (dp[i] << 1). 

After all the dynamic programming calculations are done, the program calculates the recipes by starting at the dp[target] and using ingredients to get to dp[0], saving the recipes into an array.

O(n ^ 3 * m) where n is magnium limit and m is number of ingredients in ingredient list

# Explanation on ingredients file
The ingredients file should be a text file with each row having 5 numbers corresponding to the 5 values of each ingredient.

An ingredients file containing all ingredients in the game is included as example / use. 

![Example ingredients file](https://github.com/Luicosas/Potionomics-Ingredient-Matcher/blob/main/example_ingredients_file.png)

# Compilation 
g++ -O3 ./matchpotion.cpp -o matchpotion

# Run options
./matchpotion

./matchpotion (ingredient list file name) 

./matchpotion (ingredient list file name) (cauldron magnium limit) 

./matchpotion (ingredient list file name) (cauldron magnium limit) (cauldron ingredient limit) 

./matchpotion (ingredient list file name) (cauldron magnium limit) (cauldron ingredient limit) (A) (B) (C) (D) (E) 

![Example program initialization](https://github.com/Luicosas/Potionomics-Ingredient-Matcher/blob/main/example_input.png)
