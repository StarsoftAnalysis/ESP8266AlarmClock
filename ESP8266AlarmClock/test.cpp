#include <regex>
#include <string>
using std::regex;
using std::string;
using std::sregex_token_iterator;

void test (void) {
// Delimiters are spaces (\s) and/or commas
regex re("[\\s,]+");
string s = "The White Rabbit,  is very,late.";
sregex_token_iterator it(s.begin(), s.end(), re, -1);
sregex_token_iterator reg_end;
for (; it != reg_end; ++it) {
//     std::cout << it->str() << std::endl;
//    Serial.println(it->str());
}
}
