#include<iostream>
#include<vector>
#include<array>
#include<fstream>
#include<sstream>
#include<algorithm>
#include<cassert> 

using namespace std;

vector<array<int,5>> ingredients; 

void parse_ingredients(string filename) {
    ifstream file(filename);

    if(!file.is_open()) {
        cerr << "unable to open ingredients file" << endl;
        exit(1);
    }

    int line_cnt = 0;
    string line;
    while(getline(file, line)){
        stringstream ss(line);
        array<int,5> ingredient;

        for(int i = 0; i < 5; i++) {
            if(!(ss >> ingredient[i])) {
                cerr << "not enough values on line " << line_cnt + 1 << "\n"; 
                break;
            }
            if(i == 4) {
                ingredients.push_back(ingredient);
            }
        }
        line_cnt++;
    }

    // sort the ingredients by magnium count (will result in recipies being printed in order of magnium count)
    sort(ingredients.begin(), ingredients.end(), [&](array<int,5>& a, array<int,5>& b) { 
        int asum = 0, bsum = 0;
        for(int i = 0; i < 5; i++) {
            asum += a[i];
            bsum += b[i];
        }
        return asum > bsum;
    });
    ingredients.resize(unique(ingredients.begin(), ingredients.end()) - ingredients.begin());
}

vector<int> dp;
array<int,5> potion_ratio, max_magniums;

int calc_val(array<int,5> ingredient) {
    int val = 0;
    for(int i = 0, base = 1; i < 5; i++) {
        val += base * ingredient[i];
        base *= (max_magniums[i] + 1);
    }
    return val;
}

int ingredient_limit;

void calc_dp() {
    cout << "calculating dp - numbers indicate processed ingredient\n";

    dp.assign(calc_val(max_magniums) + 1, 0);
    dp[0] = 1;

    int crn_ing = 0;
    for(array<int,5> ingredient : ingredients) {
        cerr << crn_ing++ << " ";

        bool usable = 1;
        for(int i = 0; i < 5; i++) {
            if(max_magniums[i] < ingredient[i]) {
                usable = 0;
                break;
            }
        }
        if(!usable) {
            continue;
        }
        
        int offset = calc_val(ingredient);
        for(int i = 0; i < (int)(dp.size()); i++) {
            bool valid = 1;
            for(int ii = 0, tmp = i; ii < 5; ii++) {
                int uses = tmp % (max_magniums[ii] + 1) + ingredient[ii];
                tmp /= (max_magniums[ii] + 1);
                if(uses > max_magniums[ii]) {
                    valid = 0;
                }
            }
            if(!valid) {
                continue;
            }
            
            dp[i + offset] |= (dp[i] << 1) & ((2 << ingredient_limit) - 1);
        }
    }
    cout << "\n";

    cout << "finished calculating dp\n";
}

vector<vector<array<int,2>>> recipies;

void create_recipies(int target_recipe_length, array<int,5> magnium_totals, int max_recipes_to_create = 1000) {
    vector<array<int,2>> crn_recipe;
    
    function<void(int,int,int)> continue_recipe = [&](int crn_value, int crn_ingredient, int ingredients_left) {
        if(ingredients_left == 0) {
            assert(crn_value == 0);
            recipies.push_back(crn_recipe);
            cerr << (int)(recipies.size()) << " ";
        }

        for(int i = crn_ingredient; i < (int)(ingredients.size()); i++) {
            // check if using ith ingredient is possible
            bool usable = 1;
            for(int ii = 0, t = crn_value; ii < 5; ii++) {
                if(t % (max_magniums[ii] + 1) - ingredients[i][ii] < 0) {
                    usable = 0;
                }
                t /= (max_magniums[ii] + 1);
            }
            if(!usable) {
                continue;
            }
            if( (( dp[crn_value - calc_val(ingredients[i])] >> (ingredients_left - 1) ) & 1) == 0) {
                usable = 0;
            }
            if(!usable) {
                continue;
            }

            if(crn_recipe.empty() || crn_recipe.back()[0] != i) {
                crn_recipe.push_back({i, 0});
            }
            crn_recipe.back()[1]++;
            continue_recipe(crn_value - calc_val(ingredients[i]), i, ingredients_left - 1);
            crn_recipe.back()[1]--;
            if(crn_recipe.back()[1] == 0) {
                crn_recipe.pop_back();
            }
            if((int)(recipies.size()) >= max_recipes_to_create) {
                return;
            }
        }
    };
    continue_recipe(calc_val(magnium_totals), 0, target_recipe_length);
    cerr << "\n";
}

void print_recipies(int amt) {
    cout << recipies.size() << " recipies catalogued: \n";

    for(int i = 0; i < min(amt, (int)recipies.size()); i++) {
        int total_mags = 0, total_ingredients = 0;
        array<int,5> sums_mags = {0, 0, 0, 0, 0};
        for(array<int,2> crn : recipies[i]) {
            array<int,5> crn_ingredient = ingredients[crn[0]];
            total_ingredients += crn[1];
            for(int ii = 0; ii < 5; ii++) {
                total_mags += crn_ingredient[ii] * crn[1];
                sums_mags[ii] += crn_ingredient[ii] * crn[1];
                cout << crn_ingredient[ii] << " ";
            }
            cout << "x" << crn[1] << "\n";
        }
        cout << "total mags " << total_mags << "\n";
        cout << "used ingredients " << total_ingredients << "\n";
        for(int ii : sums_mags) {
            cout << ii << " ";
        }
        cout << "\n---\n";
    }
}

int main (int argv, char* args[]){
    // checks if filename a b c d e mx_magnim mx_ingredients are given
    string filename;
    if(argv < 2){
        cout << "what is the ingredients file? please enter a filename\n";
        cin >> filename;
    } else {
        filename = args[1];
    }
    parse_ingredients(filename);

    int magnium_limit;
    if(argv < 3) {
        cout << "what is the cauldron magnium limit? please enter one number: \n";
        cin >> magnium_limit;
    } else {
        magnium_limit = stoi(args[2]);
    }
    cout << "magnium limit set as " << magnium_limit << "\n";

    if(argv < 4) {
        cout << "what is the cauldron ingredient limit? please enter one number: \n";
        cin >> ingredient_limit;
    } else {
        ingredient_limit = stoi(args[3]);
    }
    cout << "ingredients limit set as " << ingredient_limit << "\n";
    
    if(argv < 9) {
        cout << "what is the potion ratio? please enter five numbers: \n";
        for(int i = 0; i < 5; i++) {
            cin >> potion_ratio[i];
        }
    } else {
        for(int i = 0; i < 5; i++) {
            potion_ratio[i] = stoi(args[4 + i]);
        }
    }
    cout << "potion ratio set as ";
    for(int i = 0; i < 5; i++) {
        cout << potion_ratio[i] << " ";
    }
    cout << "\n";

    int potion_sum = 0;
    for(int i = 0; i < 5; i++) {
        potion_sum += potion_ratio[i];
    }
    int mx_uses = magnium_limit / potion_sum;
    for(int i = 0; i < 5; i++) {
        max_magniums[i] = mx_uses * potion_ratio[i];
    }
    cout << "max magniums set as ";
    for(int i = 0; i < 5; i++) {
        cout << max_magniums[i] << " ";
    }
    cout << "\n";

    calc_dp();

    create_recipies(ingredient_limit, max_magniums);

    print_recipies(10);

    return 0;
}
