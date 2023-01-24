#include<iostream>
#include<vector>
#include<array>
#include<fstream>
#include<sstream>
#include<algorithm>
#include<cassert> 
#include<bitset>
#include<string>

using namespace std;

struct ingredient {
    array<int,5> magniums;
    bitset<10> traits;

    ingredient(array<int,5> _magniums = {0,0,0,0,0}, bitset<10> _traits = 0) {
        magniums = _magniums;
        traits = _traits;
    }

    ingredient operator + (ingredient& r) {
        ingredient ret = *this;
        for(int i = 0; i < 5; i++) {
            ret.magniums[i] += r.magniums[i];
        }
        ret.traits |= r.traits;
        return ret;
    }

    ingredient operator - (ingredient& r) {
        ingredient ret = *this;
        for(int i = 0; i < 5; i++) {
            ret.magniums[i] -= r.magniums[i];
        }
        ret.traits &= ~r.traits;
        return ret;
    }

    string to_string() {
        string res;
        for(int ii = 0; ii < 5; ii++) {
            string v = std::to_string(magniums[ii]);
            res.append(v + " ");
        }
        for(int ii = 0; ii < 5; ii++) {
            if(((traits.to_ulong() >> (ii * 2)) & 1) == 1) {
                res.append("+1");
            } 
            if(((traits.to_ulong() >> (ii * 2 + 1)) & 1) == 1) {
                res.append("-1");
            }
            if(((traits.to_ulong() >> (ii * 2)) & 3) == 0) {
                res.append("0");
            }
            if(ii != 4) {
                res.append(" ");
            }
        }
        return res;
    }
};

vector<ingredient> read_ingredients(string filename) {
    cout << "reading ingredients file\n";

    ifstream file(filename);

    if(!file.is_open()) {
        cerr << "unable to open ingredients file" << endl;
        exit(1);
    }

    vector<ingredient> ingredients; 

    string line;
    for(int line_num = 0; getline(file, line); line_num++) {
        stringstream ss(line);

        bool valid_line = 1;

        array<int,5> magniums;
        for(int i = 0; i < 5 && valid_line; i++) {
            if(!(ss >> magniums[i])) {
                valid_line = false;
                break;
            }
        }

        bitset<10> traits = 0;
        for(int i = 0; i < 5 && valid_line; i++) {
            int val;
            if(!(ss >> val)) {
                valid_line = false;
                break;
            }
            if(val == 1) {
                traits |= 1 << (i * 2);
            } 
            if(val == -1) {
                traits |= 2 << (i * 2);
            }
        }

        if(valid_line) {
            ingredients.push_back(ingredient(magniums, traits));
        } else {
            cerr << "invalid ingredient on line " << line_num << "\n";
        }
    }
    cout << "finished reading ingredients file\n";

    return ingredients;
}

struct dp_array {
    vector<int> data;
    array<int,5> data_dimensions;
    vector<ingredient> ingredients;
    
    dp_array(array<int,5> max_magniums, vector<ingredient>& _ingredients) {
        cout << "constructing dp array\n";
        data_dimensions = max_magniums;
        data.assign(recipe_to_idx(ingredient(max_magniums, (1 << 10) - 1)) + 1, 0);
        data[0] = 1;

        cout << "dp array size at " << (int)(data.size()) << "\n";

        ingredients = _ingredients;
        for(int i = 0; i < (int)(ingredients.size()); i++) {
            cout << "adding " << i << "th ingredient\n";
            add_ingredient_to_dp(ingredients[i]);
        }
        cout << "finished constructing dp array\n";
    }

    int recipe_to_idx(ingredient loc) {
        for(int i = 0; i < 5; i++) {
            if(loc.magniums[i] > data_dimensions[i] || loc.magniums[i] < 0) {
                return -1;
            }
        }
        int idx = 0;
        for(int i = 0, base = 1; i < 5; base *= (data_dimensions[i] + 1), i++) {
            idx += base * loc.magniums[i];
        }
        return idx * (1 << 10) + (int)(loc.traits.to_ulong());
    }
    
    ingredient idx_to_recipe(int idx) {
        array<int,5> magniums;
        for(int i = 0, tmp = idx >> 10; i < 5; i++) {
            int d = tmp / (data_dimensions[i] + 1);
            magniums[i] = tmp - d * (data_dimensions[i] + 1);
            tmp = d;
        }
        int traits = idx & ((1 << 10) - 1);
        return ingredient(magniums, traits);
    }

    void add_ingredient_to_dp(ingredient ingredient_to_add) {
        int ingredient_idx = recipe_to_idx(ingredient_to_add);
        if(ingredient_idx == -1) {
            return;
        }
        for(int idx = 0; idx < (int)(data.size()); idx++) {
            int nxt_idx = recipe_to_idx(idx_to_recipe(idx) + ingredient_to_add);
            if(nxt_idx == -1) {
                continue;
            }
            data[nxt_idx] |= (data[idx] << 1);
        }
    }

    vector<int> create_recipe(array<int,5> magniums, int traits, int recipe_size) {
        ingredient target(magniums, traits);

        if(recipe_to_idx(target) == -1 || ((data[recipe_to_idx(target)] >> recipe_size) & 1) == 0) {
            return vector<int>();
        }

        vector<int> recipe(recipe_size, 0);
        int ingredient_ptr = 0, recipe_ptr = 0;
        while(recipe_ptr < recipe_size) {
            assert(ingredient_ptr < (int)(ingredients.size()));

            ingredient nxt = target - ingredients[ingredient_ptr];

            bool added_ingredient = 0;
            for(int traits = 0; traits < (1 << 10) && !added_ingredient; traits++) {
                if(recipe_to_idx(nxt) == -1) {
                    break;
                }
                if((bitset<10>(traits) & nxt.traits) != nxt.traits) {
                    continue;
                }
                if((bitset<10>(traits) | target.traits) != target.traits) {
                    continue;
                }
                if(recipe_size - recipe_ptr - 1 >= 0 && ((data[recipe_to_idx(ingredient(nxt.magniums, traits))] >> (recipe_size - recipe_ptr - 1)) & 1) == 0) {
                    continue;
                }
                recipe[recipe_ptr++] = ingredient_ptr;
                target = ingredient(nxt.magniums, traits);
                added_ingredient = 1;
            }
            assert((data[recipe_to_idx(target)] >> (recipe_size - recipe_ptr)) & 1);
            if(!added_ingredient) {
                ingredient_ptr++;
            }
            // there's a bug where sometimes it won't find the recipe without the modulo statement even tho it should
            ingredient_ptr %= ingredients.size();
        }
        return recipe;
    }
};

int main (int argv, char* args[]){
    string filename;
    if(argv < 2){
        cout << "what is the ingredients file? please enter a filename\n";
        cin >> filename;
    } else {
        filename = args[1];
    }
    vector<ingredient> ingredients = read_ingredients(filename);

    int magnium_limit;
    if(argv < 3) {
        cout << "what is the cauldron magnium limit? please enter one number: \n";
        cin >> magnium_limit;
    } else {
        magnium_limit = stoi(args[2]);
    }
    cout << "magnium limit set as " << magnium_limit << "\n";

    int ingredient_limit;
    if(argv < 4) {
        cout << "what is the cauldron ingredient limit? please enter one number: \n";
        cin >> ingredient_limit;
    } else {
        ingredient_limit = stoi(args[3]);
    }
    cout << "ingredients limit set as " << ingredient_limit << "\n";
    
    array<int,5> potion_ratio;
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

    array<int,5> max_magniums;
    for(int i = 0; i < 5; i++) {
        max_magniums[i] = mx_uses * potion_ratio[i];
    }
    cout << "max magniums set as ";
    for(int i = 0; i < 5; i++) {
        cout << max_magniums[i] << " ";
    }
    cout << "\n";

    dp_array dp(max_magniums, ingredients);

    vector<int> possible_traits(1 << 10);
    for(int i = 0; i < (int)(possible_traits.size()); i++) {
        possible_traits[i] = i;
    }
    sort(possible_traits.begin(), possible_traits.end(), [](int a, int b) {
        function<int(int)> calc_bonus = [](int traits) {
            int bonus = 0;
            for(int i = 0; i < 10; i += 2) {
                bonus += ((traits >> i) & 1) * 5;
            }
            for(int i = 1; i < 10; i += 2) {
                bonus -= ((traits >> i) & 1) * 5;
            }
            return bonus;
        };
        return calc_bonus(a) > calc_bonus(b);
    });

    int cnt = 0;
    for(int crn_trait : possible_traits) {
        for(int i = 0; i < 20 / potion_sum; i++) {
            array<int,5> tar_magniums = max_magniums;
            for(int ii = 0; ii < 5; ii++) {
                tar_magniums[i] -= potion_ratio[ii] * i;
            }
            vector<int> recipe = dp.create_recipe(tar_magniums, crn_trait, ingredient_limit);

            if(recipe.empty()) {
                continue;
            }

            for(int ii = 0, repeat_count = 0; ii < (int)(recipe.size()); ii++) {
                if(ii + 1 < (int)(recipe.size()) && recipe[ii + 1] == recipe[ii]) {
                    repeat_count++;
                    continue;
                }
                cout << ingredients[recipe[ii]].to_string() << " x" << repeat_count + 1 << "\n";
                repeat_count = 0;
            }

            ingredient totals;
            for(int ii = 0; ii < (int)(recipe.size()); ii++) {
                totals = totals + ingredients[recipe[ii]];
            }
            cout << "final result: \n";
            cout << totals.to_string() << "\n";
            cout << "\n";

            cnt++;

            if(cnt >= 10) {
                return 0;
            }
        }
    }


    return 0;
}
