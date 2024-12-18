
#include "arithmetic.h"
#include <cmath>      // ��� �������������� �������, ��������, ��� sin(), cos(), sqrt()
#include <stdexcept>  // ��� ����������, ��������, ��� std::invalid_argument

// ������� ��� ��������, �������� �� ������ ������
bool isDigit(char ch) {
    return ch >= '0' && ch <= '9';
}

// ���������� ������������ Lexem
Lexem::Lexem(LexemType t, const std::string& v, int p) : type(t), value(v), priority(p) {}

bool Lexem::isOperator() const {
    return type == OPERATOR;
}

bool Lexem::isFunction() const {
    return type == FUNCTION;
}

bool Lexem::isVariable() const {
    return type == VARIABLE;
}

bool Lexem::isNumber() const {
    return type == NUMBER;
}

bool Lexem::isUnary() const {
    return type == UNARY_MINUS;
}

bool Lexem::isParenthesis() const {
    return type == LEFT_PAREN || type == RIGHT_PAREN;
}

// ���������� ��������� ������
std::ostream& operator<<(std::ostream& os, const Lexem& lexem) {
    os << lexem.value;
    return os;
}

// ���������� ��������� �����
std::istream& operator>>(std::istream& is, Lexem& lexem) {
    std::string value;
    is >> value;

    // ������ ��� ����������� ���� �������

    if (value == "-" &&
        (lexem.value.empty() || lexem.value.back() == '(' ||
            lexem.value.back() == '+' || lexem.value.back() == '-' ||
            lexem.value.back() == '*' || lexem.value.back() == '/')) {
        lexem.type = UNARY_MINUS; // ������� �����
    }
    else if (value == "+" || value == "-" || value == "*" || value == "/") {
        lexem.type = OPERATOR; // �������� ��������
    }
    else if (value == "sin" || value == "cos" || value == "tg" || value == "ctg") {
        lexem.type = FUNCTION;
    }
    else if (value == "(") {
        lexem.type = LEFT_PAREN;
    }
    else if (value == ")") {
        lexem.type = RIGHT_PAREN;
    }
    else if ((value[0] >= 'a' && value[0] <= 'z') || (value[0] >= 'A' && value[0] <= 'Z') || value[0] == '_') {
        lexem.type = VARIABLE; // ����������
    }
    else {
        lexem.type = NUMBER; // �����
    }

    lexem.value = value;
    lexem.priority = 0;
    return is;
}

// ���������� ������ PostfixConverter
PostfixConverter::PostfixConverter(const TDynamicVector<Lexem>& tokens) : infixTokens(tokens) {}

int PostfixConverter::getPriority(const Lexem& op) const {
    if (op.isUnary()) return 3;  // ������� ����� ����� ����� ������� ���������
    if (op.value == "+" || op.value == "-") return 1;
    if (op.value == "*" || op.value == "/") return 2;
    return 0;
}

void PostfixConverter::toPostfix() {
    TDynamicVector<Lexem> stack;

    for (const Lexem& token : infixTokens) {
        if (token.isNumber() || token.isVariable()) {
            postfixTokens.push_back(token);  // ���� ��� ����� ��� ����������, ��������� � ���������
        }
        else if (token.isOperator()) {
            while (!stack.empty() && stack.back().type != LEFT_PAREN &&
                getPriority(stack.back()) >= getPriority(token)) {
                postfixTokens.push_back(stack.back());
                stack.pop_back();
            }
            stack.push_back(token);
        }
        else if (token.isUnary()) {
            stack.push_back(token);
        }
        else if (token.isParenthesis()) {
            if (token.type == LEFT_PAREN) {
                stack.push_back(token);
            }
            else {
                while (!stack.empty() && stack.back().type != LEFT_PAREN) {
                    postfixTokens.push_back(stack.back());
                    stack.pop_back();
                }
                stack.pop_back(); // ������� ����������� ������
            }
        }
    }

    while (!stack.empty()) {
        postfixTokens.push_back(stack.back());
        stack.pop_back();
    }
}

void PostfixConverter::printPostfix(std::ostream& os) const {
    for (const Lexem& token : postfixTokens) {
        os << token.value << " ";
    }
    os << std::endl;
}
// �������������� ������ � �����
double PostfixConverter::simpleStringToDouble(const std::string& str) {
    double result = 0.0;
    bool isNegative = false;
    size_t i = 0;

    // ��������� ������������� �����
    if (str[i] == '-') {
        isNegative = true;
        i++;
    }

    // ��������� ����� �����
    while (i < str.length() && isDigit(str[i])) {
        result = result * 10 + (str[i] - '0');
        i++;
    }

    // ��������� ���������� ����� � ������� �����
    if (i < str.length() && str[i] == '.') {
        i++;
        double decimalPlace = 0.1;
        while (i < str.length() && isDigit(str[i])) {
            result += (str[i] - '0') * decimalPlace;
            decimalPlace /= 10;
            i++;
        }
    }

    // ��������� ���� �����, ���� ����������
    if (isNegative) {
        result = -result;
    }

    return result;
}

// ����� ��� ���������� ���������
double PostfixConverter::evaluate() {
    TDynamicVector<double> stack;
    TDynamicVector<Variable> variables;

    for (const Lexem& token : postfixTokens) {
        if (token.isNumber()) {
            stack.push_back(simpleStringToDouble(token.value));
        }
        else if (token.isVariable()) {
            auto it = std::find_if(variables.begin(), variables.end(),
                [&token](const Variable& var) { return var.name == token.value; });

            if (it != variables.end()) {
                stack.push_back(it->value);  // ���������� �������� ����������
            }
            else {
                std::cout << "Enter value for variable " << token.value << ": ";
                double val;
                std::cin >> val;
                variables.push_back({ token.value, val });
                stack.push_back(val);  // ��������� ����������
            }
        }
        else if (token.isUnary()) {
            if (stack.empty()) throw std::invalid_argument("Insufficient arguments for unary minus");
            double operand = stack.back();
            stack.pop_back();
            stack.push_back(-operand);
        }
        else if (token.isOperator()) {
            if (stack.size() < 2) throw std::invalid_argument("Insufficient arguments for operator");
            double right = stack.back(); stack.pop_back();
            double left = stack.back(); stack.pop_back();

            if (token.value == "+") stack.push_back(left + right);
            else if (token.value == "-") stack.push_back(left - right);
            else if (token.value == "*") stack.push_back(left * right);
            else if (token.value == "/") {
                if (right == 0) throw std::invalid_argument("Division by zero");
                stack.push_back(left / right);
            }
        }
    }

    if (stack.size() != 1) throw std::invalid_argument("Invalid expression");
    return stack.back();
}