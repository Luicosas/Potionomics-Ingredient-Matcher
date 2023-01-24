//#pragma GCC optimize("O3","unroll-loops")
//#pragma GCC target("avx","avx2")
#include<bits/stdc++.h>
 
#ifdef LOCAL
#include<debug.h>
#define debug(...) cerr << "[" << #__VA_ARGS__ << "]:", debug_out(__VA_ARGS__)
#else
#define debug(...)
#endif

using namespace std;

#define pb push_back
#define ll int_fast64_t
#define sz(x) (int)(x).size()
#define SZ(x) (int)(sizeof(x) / sizeof(x[0]))
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

std::vector<std::array<int,5>> ingredients; 

void parse_ingredients(std::string filename) {
    std::ifstream file(filename);

    if(!file.is_open()) {
        std::cerr << "unable to open ingredients file" << std::endl;
        return;
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
std::array<int,5> potion, mx_mags;

int calc_val(std::array<int,5> ingredient) {
    int val = 0;
    for(int i = 0, base = 1; i < 5; i++) {
        val += base * ingredient[i];
        base *= (mx_mags[i] + 1);
    }
    return val;
}

int cauldron_ingredient_limit;

void calc_dp() {
    debug("calculating dp");

    dp.assign(calc_val(mx_mags) + 1, 0);
    dp[0] = 1;

    int crn_ing = 0;
    for(std::array<int,5> ingredient : ingredients) {
        std::cerr << crn_ing++ << " ";

        bool usable = 1;
        for(int i = 0; i < 5; i++) {
            if(mx_mags[i] < ingredient[i]) {
                usable = 0;
                break;
            }
        }
        if(!usable) {
            continue;
        }
        
        int offset = calc_val(ingredient);
        for(int i = 0; i < sz(dp); i++) {
            bool valid = 1;
            for(int ii = 0, tmp = i; ii < 5; ii++) {
                int uses = tmp % (mx_mags[ii] + 1) + ingredient[ii];
                tmp /= (mx_mags[ii] + 1);
                if(uses > mx_mags[ii]) {
                    valid = 0;
                }
            }
            if(!valid) {
                continue;
            }
            
            dp[i + offset] |= (dp[i] << 1) & ((2 << cauldron_ingredient_limit) - 1);
        }
    }
    std::cerr << "\n";
}

std::vector<std::vector<array<int,2>>> recipies;

void create_recipies(int mx_ingredients) {
    debug("creating recipies");

    int offset = calc_val(potion);

    for(int i = sz(dp) - 1; i > 0; i -= offset) {
        for(int ii = mx_ingredients; ii > 0; ii--) {
            if(((dp[i] >> ii) & 1) == 0) {
                continue;
            }
            recipies.push_back(vector<array<int,2>>());
            
            int remaining = ii, crn_idx = i; 
            for(int iii = 0; iii < sz(ingredients); iii++) {
                bool skip = 0;
                for(int iiii = 0; iiii < 5; iiii++) {
                    if(ingredients[iii][iiii] > mx_mags[iiii]) {
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
                        if(tmp % (mx_mags[iiii] + 1) < ingredients[iii][iiii]) {
                            valid = 0;
                        }
                        tmp /= (mx_mags[iiii] + 1);
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
        cout << "mags " << total_mags << "\n";
        cout << "ingredients " << total_ingredients << "\n";
        for(int ii : sums_mags) {
            cout << ii << " ";
        }
        cout << "\n---\n";
    }
}

int main (int argv, char* args[]){
    // checks if filename a b c d e mx_magnim mx_ingredients are given
    if(argv < 2){
        std::cerr << "not enough parameters given" << std::endl;
        return 1;
    }
    parse_ingredients(args[1]);
    
    if(argv < 7) {
        cout << "what is the potion ratio? please enter five numbers: \n";
        for(int i = 0; i < 5; i++) {
            cin >> potion[i];
        }
    } else {
        for(int i = 0; i < 5; i++) {
            potion[i] = stoi(args[2 + i]);
        }
    }

    int cauldron_mag_limit;
    if(argv < 8) {
        cout << "what is the cauldron magnium limit? please enter one number: \n";
        cin >> cauldron_mag_limit;
    } else {
        cauldron_mag_limit = stoi(args[7]);
    }

    if(argv < 9) {
        cout << "what is the cauldron ingredient limit? please enter one number: \n";
        cin >> cauldron_ingredient_limit;
    } else {
        cauldron_ingredient_limit = stoi(args[8]);
    }

    int potion_sum = 0;
    for(int i = 0; i < 5; i++) {
        potion_sum += potion[i];
    }
    int mx_uses = cauldron_mag_limit / potion_sum;
    for(int i = 0; i < 5; i++) {
        mx_mags[i] = mx_uses * potion[i];
    }

    calc_dp();

    create_recipies(cauldron_ingredient_limit);

    print_best_recipies(3);

    return 0;
}
