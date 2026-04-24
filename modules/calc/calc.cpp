#include "calc.hpp"
#include "../syscalls/syscalls.hpp"
#include "../shell/shell.hpp"
#include "../string/string.hpp"

static int parse_int(const char *str)
{
    if (!str)
        return 0;

    int sign = 1;
    int result = 0;

    if (*str == '-')
    {
        sign = -1;
        str++;
    }
    else if (*str == '+')
    {
        str++;
    }

    while (*str >= '0' && *str <= '9')
    {
        result = result * 10 + (*str - '0');
        str++;
    }

    return sign * result;
}

void calc_init(char *args)
{
    if (args)
    {
        char *num1_str = args;
        char *op_str = find_space(num1_str);
        if (!op_str)
        {
            ksys_print("Usage: calc <num1> <op> <num2>\n");
            return;
        }
        *op_str = '\0';
        op_str++;

        char *num2_str = find_space(op_str);
        if (!num2_str)
        {
            ksys_print("Usage: calc <num1> <op> <num2>\n");
            return;
        }
        *num2_str = '\0';
        num2_str++;

        int num1 = parse_int(num1_str);
        int num2 = parse_int(num2_str);
        char op = op_str[0];

        switch (op)
        {
        case '+':
            calc_add(num1, num2);
            break;
        case '-':
            calc_sub(num1, num2);
            break;
        case '*':
            calc_mul(num1, num2);
            break;
        case '/':
            calc_div(num1, num2);
            break;
        default:
            ksys_print("Unknown operator: ");
            ksys_putchar(op);
            ksys_putchar('\n');
            break;
        }
    }
    else
    {
        ksys_print("Usage: calc <num1> <op> <num2>\n");
    }
}

void calc_add(int a, int b)
{
    int result = a + b;
    ksys_print("Result: ");
    print_uint32(result);
    ksys_putchar('\n');
}

void calc_sub(int a, int b)
{
    int result = a - b;
    ksys_print("Result: ");
    print_uint32(result);
    ksys_putchar('\n');
}

void calc_mul(int a, int b)
{
    int result = a * b;
    ksys_print("Result: ");
    print_uint32(result);
    ksys_putchar('\n');
}

void calc_div(int a, int b)
{
    if (b == 0)
    {
        ksys_print("Error: Division by zero is not allowed.\n");
        return;
    }

    int result = a / b;
    ksys_print("Result: ");
    print_uint32(result);
    ksys_putchar('\n');
}