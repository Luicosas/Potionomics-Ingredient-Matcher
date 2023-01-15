#include<iostream>
#include<vector>
#include<array>
#include<fstream>
#include<sstream>
#include<algorithm>
 
using namespace std;

std::vector<std::array<int,5>> ingredients; 

void parse_ingredients(std::string filename) {
    std::ifstream file(filename);

    if(!file.is_open()) {
        std::cerr << "unable to open ingredients file" << std::endl;
        exit(1);
    }

    int line_cnt = 0;
    std::string line;
    while(getline(file, line)){
        std::stringstream ss(line);
        std::array<int,5> ingredient;

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
    sort(ingredients.begin(), ingredients.end(), [&](std::array<int,5>& a, std::array<int,5>& b) { 
        int asum = 0, bsum = 0;
        for(int i = 0; i < 5; i++) {
            asum += a[i];
            bsum += b[i];
        }
        return asum > bsum;
    });
    ingredients.resize(unique(ingredients.begin(), ingredients.end()) - ingredients.begin());
}

std::vector<int> dp;
std::array<int,5> potion_ratio, max_magniums;

int calc_val(std::array<int,5> ingredient) {
    int val = 0;
    for(int i = 0, base = 1; i < 5; i++) {
        val += base * ingredient[i];
        base *= (max_magniums[i] + 1);
    }
    return val;
}

int ingredient_limit;

void calc_dp() {
    std::cout << "calculating dp - numbers indicate processed ingredient\n";

    dp.assign(calc_val(max_magniums) + 1, 0);
    dp[0] = 1;

    int crn_ing = 0;
    for(std::array<int,5> ingredient : ingredients) {
        std::cerr << crn_ing++ << " ";

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
    std::cout << "\n";
}

std::vector<std::vector<array<int,2>>> recipies;

void create_recipies(int mx_ingredients) {
    cout << "creating recipies\n";

    int offset = calc_val(potion_ratio);

    for(int i = (int)(dp.size()) - 1; i > 0; i -= offset) {
        for(int ii = mx_ingredients; ii > 0; ii--) {
            if(((dp[i] >> ii) & 1) == 0) {
                continue;
            }
            recipies.push_back(vector<array<int,2>>());
            
            int remaining = ii, crn_idx = i; 
            for(int iii = 0; iii < (int)(ingredients.size()); iii++) {
                bool skip = 0;
                for(int iiii = 0; iiii < 5; iiii++) {
                    if(ingredients[iii][iiii] > max_magniums[iiii]) {
                        skip = 1;
                    }
                }
                if(skip) {
                    continue;
                }

                int crn_val = calc_val(ingredients[iii]);
                while((remaining > 0) && (crn_idx - crn_val >= 0) && (((dp[crn_idx - crn_val] >> (remaining - 1)) & 1) == 1)) {
                    bool valid = 1;
                    for(int iiii = 0, tmp = crn_idx; iiii < 5; iiii++) {
                        if(tmp % (max_magniums[iiii] + 1) < ingredients[iii][iiii]) {
                            valid = 0;
                        }
                        tmp /= (max_magniums[iiii] + 1);
                    }
                    if(!valid) {
                        break;
                    }

                    if(recipies.back().empty() || recipies.back().back()[0] != iii) {
                        recipies.back().push_back({iii, 0});
                    }
                    recipies.back().back()[1]++;
                    remaining--;
                    crn_idx -= crn_val;
                }
            }
        }
    }
}

void print_best_recipies(int amt) {
    cout << recipies.size() << " recipies found: \n";

    for(int i = 0; i < min(amt, (int)recipies.size()); i++) {
        int total_mags = 0, total_ingredients = 0;
        std::array<int,5> sums_mags = {0, 0, 0, 0, 0};
        for(std::array<int,2> crn : recipies[i]) {
            std::array<int,5> crn_ingredient = ingredients[crn[0]];
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
    std::string filename;
    if(argv < 2){
        std::cout << "what is the ingredients file? please enter a filename\n";
        std::cin >> filename;
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

    create_recipies(ingredient_limit);

    print_best_recipies(10);

    return 0;
}
