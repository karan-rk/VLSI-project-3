#include <iostream>  
#include <vector>    
#include <string>    
#include <algorithm> 
#include <map>       
#include <set>      
#include <sstream>
#include <cmath>   
#include <bitset>
using namespace std;


bool isCovered(const string& minterm, const string& implicant) {
    // Check each bit in the minterm against the implicant
    for (size_t i = 0; i < minterm.size(); ++i) {
        // If implicant bit is not '-' and does not match minterm bit, return false
        if (implicant[i] != '-' && implicant[i] != minterm[i]) {
            return false;  // Implicant does not cover the minterm at this bit
        }
    }
    return true;  // All bits match or are covered by don't care positions in implicant
}

// Converts an integer to a binary string with a specified length
string toBinary(int number, int digits) {
    string binary = bitset<32>(number).to_string();
    return binary.substr(32 - digits);  // adjust length as necessary
}

vector<string> findEPI(const map<string, vector<string>>& chart) {
    vector<string> essentialPrimeImplicants;
    // Iterate over each minterm's list of implicants
    for (const auto& minterm : chart) {
        if (minterm.second.size() == 1) {  // If exactly one implicant covers this minterm
            string soleImplicant = minterm.second.front();
            if (find(essentialPrimeImplicants.begin(), essentialPrimeImplicants.end(), soleImplicant) == essentialPrimeImplicants.end()) {
                essentialPrimeImplicants.push_back(soleImplicant);
            }
        }
    }
    return essentialPrimeImplicants;
}


vector<string> findVariables(const string& minterm) {
    vector<string> varList;
    for (size_t i = 0; i < minterm.length(); ++i) {
        if (minterm[i] == '0') {
            varList.push_back(string(1, static_cast<char>(i + 'A')) + "'");
        }
        else if (minterm[i] == '1') {
            varList.push_back(string(1, static_cast<char>(i + 'A')));
        }
    }
    return varList;
}

// Function to group minterms by count of 1's in their binary representation
map<int, vector<string>> groupMinterms(const vector<string>& binaryMinterms) {
    map<int, vector<string>> groups;

    for (string minterm : binaryMinterms) {
        int countOnes = count(minterm.begin(), minterm.end(), '1');
        groups[countOnes].push_back(minterm);
    }

    // Debug: print groups for verification
    cout << "Grouping based on count of '1's in binary representation:\n";
    for (const auto& group : groups) {
        cout << "Group " << group.first << " (" << group.second.size() << " elements): ";
        for (const auto& bin : group.second) {
            cout << bin << " ";
        }
        cout << endl;
    }

    return groups;
}

// Combines two minterms if they differ by exactly one bit, using a dash '-' to mark the differing bit.
pair<bool, string> tryCombine(const string& a, const string& b) {
    if (a.length() != b.length()) {
        return { false, "" };  // Ensure both strings are of equal length
    }

    int countDifferences = 0;
    string result = a;

    for (size_t i = 0; i < a.length(); ++i) {
        if (a[i] != b[i]) {
            countDifferences++;
            result[i] = '-';
            if (countDifferences > 1) {
                return { false, "" };  // Not combinable if more than one difference
            }
        }
    }

    return countDifferences == 1 ? make_pair(true, result) : make_pair(false, "");
}


// Function to process groups to find prime implicants
set<string> processGroups(map<int, vector<string>>& groups) {
    set<string> primeImplicants;
    map<int, vector<string>> newGroups;
    bool progress = false;
    int count = 0;
    while (true) {
        progress = false;
        newGroups.clear();

        for (auto it = groups.begin(); it != groups.end(); ++it) {
            auto next = std::next(it);
            if (next == groups.end()) break;

            for (const auto& current : it->second) {
                for (const auto& nextMinterm : next->second) {
                    auto combineResult = tryCombine(current, nextMinterm);
                    if (combineResult.first) {
                        // Check if this new minterm is already added to avoid duplicates
                        if (!newGroups[it->first - 1].empty() &&
                            find(newGroups[it->first - 1].begin(), newGroups[it->first - 1].end(), combineResult.second) == newGroups[it->first - 1].end()) {
                            newGroups[it->first - 1].push_back(combineResult.second);
                        }
                        else if (newGroups[it->first - 1].empty()) {
                            newGroups[it->first - 1].push_back(combineResult.second);
                        }
                        progress = true;
                    }
                }
            }
        }

        // If no progress can be made, break and add remaining minterms as prime implicants
        if (!progress) {
            for (const auto& group : groups) {
                for (const auto& minterm : group.second) {
                    // Ensure no duplicates
                    if (primeImplicants.find(minterm) == primeImplicants.end()) {
                        primeImplicants.insert(minterm);
                    }
                }
            }
            break;
        }

        // Set up groups for the next iteration
        count++;
        cout << "Grouping " << count << ": " << endl;
        for (const auto& group : groups) {
            cout << "Group " << group.first << " (Count " << group.second.size() << "): ";
            for (const auto& bin : group.second) {
                cout << bin << " ";
            }
            cout << "\n";
        }
        groups = newGroups;
    }

    return primeImplicants;
}

string binaryToExpression(const string& binary) {
    string result;
    bool firstTerm = true;  // Flag to manage spacing and formatting

    for (size_t i = 0; i < binary.size(); ++i) {
        if (binary[i] == '1' || binary[i] == '0') {
            if (!firstTerm) {
                result += " ";  // Add space before each term except the first
            }
            result += char('A' + i);  // Append the variable
            if (binary[i] == '0') {
                result += "'";  // Append ' for negated terms
            }
            firstTerm = false;  // Update flag after adding the first term
        }
    }

    return result;
}

vector<int> readNumbers(const string& prompt) {
    cout << prompt;
    string line;
    getline(cin, line);
    istringstream iss(line);
    set<int> numbers;
    int num;
    while (iss >> num) {
        if (iss.fail() || num < 0) {
            iss.clear(); // Clear error state
            cout << "Invalid input. Please enter only non-negative integers.\n";
            getline(cin, line); // Re-prompt input
            iss.str(line);
            continue;
        }
        numbers.insert(num);
    }
    return vector<int>(numbers.begin(), numbers.end());
}

int findMaxBits(const vector<int>& numbers) {
    if (numbers.empty()) return 0;
    int maxNum = *max_element(numbers.begin(), numbers.end());
    return ceil(log2(maxNum + 1));
}

map<string, vector<string>> flipChart(map<string, vector<string>> original) {
    map<string, vector<string>> fchart;
    set<string> implicantsToAdd;
    for (auto pair : original) {
        for (string imp : pair.second) {
            implicantsToAdd.insert(imp);
        }
    }
    for (string imp : implicantsToAdd) {
        for (auto pair : original) {
            if (find(pair.second.begin(), pair.second.end(), imp) != pair.second.end()) fchart[imp].push_back(pair.first);
        }
    }
    return fchart;
}

map<string, vector<string>> Reduce(const map<string, vector<string>>& chart) {
    map<string, vector<string>> reducedChart = chart;
    
    //Col dominance: (if a minterm has same implicants as another and more it can be removed)
    set<string> keysToRemove;
    for (auto ci : reducedChart) {
        set<string> ciImplicants(ci.second.begin(), ci.second.end());
        for (auto cj : reducedChart) {
            if (ci.first != cj.first) {
                set<string> cjImplicants(cj.second.begin(), cj.second.end());
                if (keysToRemove.find(cj.first) == keysToRemove.end() && !cj.second.empty() && includes(ciImplicants.begin(), ciImplicants.end(), cjImplicants.begin(), cjImplicants.end())) { //if the implicants of cj are a subset of ci:
                    keysToRemove.insert(ci.first);
                    break;
                }
            }
        }
    }
    for (string key : keysToRemove) {
        reducedChart.erase(key);
    }
    keysToRemove.clear();

    //Row dominance:
    //first create flipped chart:
    set<string> pitoremove;
    map<string, vector<string>> fchart = flipChart(reducedChart);
    for (auto ri : fchart) {
        set<string> riMinterms(ri.second.begin(), ri.second.end());
        for (auto rj : fchart) {
            if (ri.first != rj.first) {
                set<string> rjMinterms(rj.second.begin(), rj.second.end());
                if (keysToRemove.find(ri.first) == keysToRemove.end() && !ri.second.empty() && includes(riMinterms.begin(), riMinterms.end(), rjMinterms.begin(), rjMinterms.end())) pitoremove.insert(rj.first); break;
            }
        }
    }
    for (string key : pitoremove) {
        for (auto &pair : reducedChart) {
            auto rem = find(pair.second.begin(), pair.second.end(), key);
            if (rem != pair.second.end()) {
                pair.second.erase(rem);
            }
        }
    }

    return reducedChart;

}

int MIS_quick(const map<string, vector<string>>& chart) {
    map<string, vector<string>> chartcopy = chart;
    set<string> keys;
    for (auto pair : chartcopy) {
        if (pair.second.empty()) keys.insert(pair.first);
    }
    for (string key : keys) {
        chartcopy.erase(key);
    }
    map<string, vector<string>> MIS;
    do {
        int minimplicants = INT_MAX;
        string key = "";
        for (const auto& pair : chartcopy) {
            if (pair.second.size() < minimplicants) {
                minimplicants = pair.second.size();
                key = pair.first;
            }
        }
        if(chartcopy.size() != 0) MIS[key] = chartcopy[key];
        vector<string> keysToRemove;
        for (string implicant : MIS[key]) {
            for (const auto& pair : chartcopy) {
                if (find(pair.second.begin(), pair.second.end(), implicant) != pair.second.end()) {//if a minterm is convered by implicant, remove
                    //chartcopy.erase(pair.first);
                    keysToRemove.push_back(pair.first);
                }
            }
            for (auto key : keysToRemove) {
                chartcopy.erase(key);
            }
        }
    } while (!chartcopy.empty());

    return MIS.size();
    
}

int Cost(vector<string> currentSolution) {
    if (currentSolution.empty()) return 0;
    else {
        /*set<int> indicies;
        for (string s : currentSolution) {
            for (int i = 0; i < s.size(); i++) {
                if (s.at(i) == '1' || s.at(i) == '0') indicies.insert(i);
            }
        }
        return indicies.size();
        */
        return currentSolution.size();
    }

}

vector<string> BestSolution(vector<string> solution1, vector<string> solution2) {
    if (solution1.at(0) == "no solution") return solution2;
    else if (solution2.at(0) == "no solution") return solution1;
    else return Cost(solution1) >= Cost(solution2) ? solution1 : solution2;
}

bool isTerminal(map<string, vector<string>> chart) {
    bool flag = true;
    for (auto pair : chart) {
        if (!pair.second.empty()) flag = false;
    }
    return flag;
}

vector<string> BCP(map<string, vector<string>> &chart, int U, vector<string> &currentSolution) {
    map<string, vector<string>> rchart = Reduce(chart);
    
    if (isTerminal(rchart)) { //if terminal case
        if (Cost(currentSolution) < U) {
            U = Cost(currentSolution);
            if (currentSolution.empty()) return { "no solution" };
            else return currentSolution;
        }
        else {
            return { "no solution." };
        }
    }
    int LB = MIS_quick(rchart);
    if (LB >= U) {
        return { "no solution" };
    }
    
    string pi;
    map<string, vector<string>> rchart1 = rchart;
    map<string, vector<string>> rchart0 = rchart;
    for (auto pair : rchart) {
        if (!pair.second.empty()) {
            pi = pair.second.at(0); //choose a pi
            break;
        }
    }
    
    //set pi to 1 and branch (any chart entry with pi becomes (1 + pj), therfore remove these entries completely and add to currentSolution)
    set<string> keys;
    for (auto pair : rchart1) {
        for (string s : pair.second) {
            if (s == pi) keys.insert(pair.first); break;
        }
    }
    for (auto &pair : rchart1) {
        if (keys.find(pair.first) != keys.end()) {
            pair.second.clear();
        }
    }
    keys.clear();
    vector<string> cs1 = currentSolution;
    cs1.push_back(pi);
    auto s1 = BCP(rchart1, U, cs1);
    if (Cost(s1) == LB) return s1;
    //set pi to 0 and branch (remove pi (0 + pj) and DONT add to current solution)
    for (auto &pair : rchart0) {
        pair.second.erase(remove_if(pair.second.begin(), pair.second.end(), [&pi](string s) {return s == pi; }), pair.second.end()); //remove pi 
    }
    auto s0 = BCP(rchart0, U, currentSolution);
    return BestSolution(s0, s1);
}

int main() {
    
    vector<int> minterms = readNumbers("Enter the minterms (space-separated): ");
    //vector<int> dontCares = readNumbers("Enter the don't cares (space-separated, if any): ");
    vector<int> dontCares;
    // Combine minterms and don't cares into allMinterms
    vector<int> allMinterms(minterms);
    allMinterms.insert(allMinterms.end(), dontCares.begin(), dontCares.end());

    // Determine the maximum number of bits for binary representation
    int maxBits = findMaxBits(allMinterms);
    vector<string> binaryMinterms;

    // Output binary representations of all minterms
    cout << "All minterms and their binary representations:\n";
    for (int minterm : allMinterms) {
        string binary = toBinary(minterm, maxBits);
        binaryMinterms.push_back(binary);
        cout << minterm << " -> " << binary << endl;
    }

    // Group minterms
    auto groups = groupMinterms(binaryMinterms);

    // Identify prime implicants
    auto primeImplicants = processGroups(groups);
    map<string, vector<string>> varImplicants;
    cout << "Prime Implicants:\n";
    for (const auto& implicant : primeImplicants) {
        vector<string> vars = findVariables(implicant);
        varImplicants[implicant] = vars; // Map binary implicant to its variable representation
        cout << implicant << " -> " << binaryToExpression(implicant) << " (";
        for (const auto& var : vars) {
            cout << var << " ";
        }
        cout << ")\n";
    }

    // Create a prime implicant chart
    map<string, vector<string>> chart;
    for (int minterm : allMinterms) {
        string binaryMinterm = toBinary(minterm, maxBits);
        for (const auto& implicant : primeImplicants) {
            if (isCovered(binaryMinterm, implicant)) {
                chart[binaryMinterm].push_back(implicant); //chart consists of each prime implicant with it's minterms(binary)
            }
        }
    }

    // Extract essential prime implicants
    vector<string> epiVector = findEPI(chart);
    set<string> essentialPrimeImplicants(epiVector.begin(), epiVector.end());

    set<string> coveredMinterms; // To track minterms covered by EPIs

    // Initialize allMinterms set for Branch-and-Bound
    set<string> allMintermBins; // Collecting all minterms that need to be covered
    for (const auto& pair : chart) {
        allMintermBins.insert(pair.first);
    }

    // Initialize an upper bound U for Branch-and-Bound
    int U = INT_MAX; // Start with the maximum possible cost

    // Call Branch-and-Bound
    vector<string> currentSolution;
    vector<string> best;
    
    best = BCP(chart, U, currentSolution);

    // Display results
    cout << "Essential Prime Implicants:\n";
    for (const auto& epi : essentialPrimeImplicants) {
        cout << epi << endl;
    }

    cout << "Minimal Covering Set of Prime Implicants:\n";
    for (const auto& implicant : best) {
        cout << implicant << endl;
    }

    // Display results in Boolean expression format
    cout << "Essential Prime Implicants as Boolean Expressions:\n";
    string booleanExpression;
    for (const auto& epi : essentialPrimeImplicants) {
        if (!booleanExpression.empty()) booleanExpression += " + ";
        booleanExpression += binaryToExpression(epi);
    }
    cout << booleanExpression << endl;

    return 0;
}
