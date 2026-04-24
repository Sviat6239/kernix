#include "calc.hpp"
#include "../syscalls/syscalls.hpp"
#include "../string/string.hpp"
#include "../shell/shell.hpp"

static bool is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static void skip_spaces(const char *&p)
{
    while (*p && is_space(*p))
        p++;
}

static bool parse_int_token(const char *&p, int &value)
{
    skip_spaces(p);

    int sign = 1;
    if (*p == '-')
    {
        sign = -1;
        p++;
    }
    else if (*p == '+')
    {
        p++;
    }

    if (*p < '0' || *p > '9')
        return false;

    int result = 0;
    while (*p >= '0' && *p <= '9')
    {
        result = result * 10 + (*p - '0');
        p++;
    }

    value = result * sign;
    return true;
}

static bool parse_expression(const char *args, int &left, char &op, int &right)
{
    if (!args)
        return false;

    const char *p = args;
    if (!parse_int_token(p, left))
        return false;

    skip_spaces(p);
    if (*p != '+' && *p != '-' && *p != '*' && *p != '/')
        return false;

    op = *p;
    p++;

    if (!parse_int_token(p, right))
        return false;

    skip_spaces(p);
    return *p == '\0';
}

static void print_int32(int value)
{
    if (value == 0)
    {
        ksys_putchar('0');
        return;
    }

    unsigned int number;
    if (value < 0)
    {
        ksys_putchar('-');
        number = static_cast<unsigned int>(-(static_cast<long long>(value)));
    }
    else
    {
        number = static_cast<unsigned int>(value);
    }

    char buffer[11];
    int index = 0;

    while (number > 0)
    {
        buffer[index++] = static_cast<char>('0' + (number % 10));
        number /= 10;
    }

    while (index > 0)
        ksys_putchar(buffer[--index]);
}

static void print_result(int value)
{
    ksys_print("Result: ");
    print_int32(value);
    ksys_putchar('\n');
}

static void print_scaled_result(int scaled, int scale)
{
    ksys_print("Result: ");

    if (scaled < 0)
    {
        ksys_putchar('-');
        scaled = -scaled;
    }

    int int_part = scaled / scale;
    int frac_part = scaled % scale;

    print_int32(int_part);
    ksys_putchar('.');

    int p = scale / 10;
    while (p > 0)
    {
        int digit = frac_part / p;
        ksys_putchar(static_cast<char>('0' + digit));
        frac_part %= p;
        p /= 10;
    }

    ksys_putchar('\n');
}

static bool checked_mul_int(int a, int b, int &out)
{
    static constexpr int INT32_MAX_VAL = 2147483647;
    static constexpr int INT32_MIN_VAL = (-2147483647 - 1);

    if (a == 0 || b == 0)
    {
        out = 0;
        return true;
    }

    if (a == -1 && b == INT32_MIN_VAL)
        return false;
    if (b == -1 && a == INT32_MIN_VAL)
        return false;

    if (a > 0)
    {
        if (b > 0 && a > INT32_MAX_VAL / b)
            return false;
        if (b < 0 && b < INT32_MIN_VAL / a)
            return false;
    }
    else
    {
        if (b > 0 && a < INT32_MIN_VAL / b)
            return false;
        if (b < 0 && a != 0 && b < INT32_MAX_VAL / a)
            return false;
    }

    out = a * b;
    return true;
}

static constexpr int FP = 10000;
static constexpr int PI_FP = 31416;
static constexpr int HALF_PI_FP = 15708;
static constexpr int TWO_PI_FP = 62832;

static int deg_to_rad_fp(int degrees)
{
    return (degrees * PI_FP) / 180;
}

static int normalize_rad_fp(int x)
{
    while (x > PI_FP)
        x -= TWO_PI_FP;
    while (x < -PI_FP)
        x += TWO_PI_FP;
    return x;
}

static int sin_fp(int x)
{
    x = normalize_rad_fp(x);

    int term = x;
    int sum = x;
    for (int k = 1; k <= 6; ++k)
    {
        int denom = (2 * k) * (2 * k + 1);
        term = -(term * x / FP) * x / FP;
        term /= denom;
        sum += term;
    }

    return sum;
}

static int cos_fp(int x)
{
    x = normalize_rad_fp(x);

    int term = FP;
    int sum = FP;
    for (int k = 1; k <= 6; ++k)
    {
        int denom = (2 * k - 1) * (2 * k);
        term = -(term * x / FP) * x / FP;
        term /= denom;
        sum += term;
    }

    return sum;
}

static int atan_fp_core(int z)
{
    bool neg = z < 0;
    if (neg)
        z = -z;

    int result;
    if (z <= FP)
    {
        int zpow = z;
        result = zpow;
        for (int n = 1; n <= 8; ++n)
        {
            zpow = (zpow * z / FP) * z / FP;
            int term = zpow / (2 * n + 1);
            if (n & 1)
                result -= term;
            else
                result += term;
        }
    }
    else
    {
        int inv = (FP * FP) / z;
        result = HALF_PI_FP - atan_fp_core(inv);
    }

    return neg ? -result : result;
}

static int rad_to_deg_fp(int radians_fp)
{
    return (radians_fp * 180) / PI_FP;
}

static bool is_help_request(const char *args)
{
    if (!args)
        return false;

    const char *p = args;
    skip_spaces(p);

    if (*p == '\0')
        return false;

    const char *start = p;
    while (*p && !is_space(*p))
        p++;

    int len = static_cast<int>(p - start);
    skip_spaces(p);
    if (*p != '\0')
        return false;

    if (len == 4 &&
        start[0] == 'h' &&
        start[1] == 'e' &&
        start[2] == 'l' &&
        start[3] == 'p')
    {
        return true;
    }

    if (len == 2 && start[0] == '-' && start[1] == 'h')
        return true;

    if (len == 6 &&
        start[0] == '-' &&
        start[1] == '-' &&
        start[2] == 'h' &&
        start[3] == 'e' &&
        start[4] == 'l' &&
        start[5] == 'p')
    {
        return true;
    }

    return false;
}

static void calc_print_help()
{
    ksys_print("Calculator help\n");
    ksys_print("Usage:\n");
    ksys_print("  calc <num1> <op> <num2>\n");
    ksys_print("  calc help | calc -h | calc --help\n");
    ksys_print("\n");
    ksys_print("Supported operators in shell command:\n");
    ksys_print("  +  Addition\n");
    ksys_print("  -  Subtraction\n");
    ksys_print("  *  Multiplication\n");
    ksys_print("  /  Division\n");
    ksys_print("\n");
    ksys_print("Input examples:\n");
    ksys_print("  calc 2+3\n");
    ksys_print("  calc 10 / 2\n");
    ksys_print("  calc -7 * 5\n");
    ksys_print("  calc 12 - -4\n");
    ksys_print("\n");
    ksys_print("Other implemented calc API functions (module-level):\n");
    ksys_print("  sqr, pow, root, log, log10\n");
    ksys_print("  sin, cos, tan, ctg, asin, acos, atan, actg\n");
    ksys_print("  factorial, tetration, fibonacci\n");
}

void calc_init(char *args)
{
    int left = 0;
    int right = 0;
    char op = 0;

    if (is_help_request(args))
    {
        calc_print_help();
        return;
    }

    if (!parse_expression(args, left, op, right))
    {
        ksys_print("Usage: calc <num1> <op> <num2>\n");
        ksys_print("Try: calc help\n");
        return;
    }

    switch (op)
    {
    case '+':
        calc_add(left, right);
        break;
    case '-':
        calc_sub(left, right);
        break;
    case '*':
        calc_mul(left, right);
        break;
    case '/':
        calc_div(left, right);
        break;
    default:
        ksys_print("Unknown operator: ");
        ksys_putchar(op);
        ksys_putchar('\n');
        break;
    }
}

void calc_add(int a, int b)
{
    print_result(a + b);
}

void calc_sub(int a, int b)
{
    print_result(a - b);
}

void calc_mul(int a, int b)
{
    print_result(a * b);
}

void calc_div(int a, int b)
{
    if (b == 0)
    {
        ksys_print("Error: Division by zero is not allowed.\n");
        return;
    }

    int result = a / b;
    print_result(result);
}

void calc_sqr(int a)
{
    int result = a * a;
    ksys_print("Result: ");
    print_uint32(result);
    ksys_putchar('\n');
}

void calc_pow(int base, int exp)
{
    int result = 1;
    for (int i = 0; i < exp; i++)
    {
        result *= base;
    }
    ksys_print("Result: ");
    print_uint32(result);
    ksys_putchar('\n');
}

void calc_root(int a, int n)
{
    if (n <= 0)
    {
        ksys_print("Error: Root degree must be positive.\n");
        return;
    }

    int result = 1;
    int low = 1;
    int high = a;

    while (low <= high)
    {
        int mid = (low + high) / 2;
        int mid_pow = 1;
        for (int i = 0; i < n; i++)
        {
            mid_pow *= mid;
        }

        if (mid_pow == a)
        {
            result = mid;
            break;
        }
        else if (mid_pow < a)
        {
            low = mid + 1;
            result = mid;
        }
        else
        {
            high = mid - 1;
        }
    }

    ksys_print("Result: ");
    print_uint32(result);
    ksys_putchar('\n');
}

void calc_log(int a, int base)
{
    if (a <= 0 || base <= 1)
    {
        ksys_print("Error: Invalid input for logarithm.\n");
        return;
    }

    int result = 0;
    int power = 1;

    while (power < a)
    {
        power *= base;
        result++;
    }

    if (power > a)
        result--;

    ksys_print("Result: ");
    print_uint32(result);
    ksys_putchar('\n');
}

void calc_log10(int a)
{
    calc_log(a, 10);
}

void calc_sin(int angle)
{
    int radians = deg_to_rad_fp(angle);
    int result = sin_fp(radians);
    print_scaled_result(result, FP);
}

void calc_cos(int angle)
{
    int radians = deg_to_rad_fp(angle);
    int result = cos_fp(radians);
    print_scaled_result(result, FP);
}

void calc_tan(int angle)
{
    int radians = deg_to_rad_fp(angle);
    int s = sin_fp(radians);
    int c = cos_fp(radians);

    if (c == 0 || c == 1 || c == -1)
    {
        ksys_print("Error: tan is undefined for this angle.\n");
        return;
    }

    int result = (s * FP) / c;
    print_scaled_result(result, FP);
}

void calc_ctg(int angle)
{
    int radians = deg_to_rad_fp(angle);
    int s = sin_fp(radians);
    int c = cos_fp(radians);

    if (s == 0 || s == 1 || s == -1)
    {
        ksys_print("Error: ctg is undefined for this angle.\n");
        return;
    }

    int result = (c * FP) / s;
    print_scaled_result(result, FP);
}

void calc_asin(int value)
{
    if (value < -1 || value > 1)
    {
        ksys_print("Error: asin input must be -1, 0 or 1 in current integer mode.\n");
        return;
    }

    if (value == -1)
    {
        print_scaled_result(-900000, FP);
        return;
    }
    if (value == 0)
    {
        print_scaled_result(0, FP);
        return;
    }

    print_scaled_result(900000, FP);
}

void calc_acos(int value)
{
    if (value < -1 || value > 1)
    {
        ksys_print("Error: acos input must be -1, 0 or 1 in current integer mode.\n");
        return;
    }

    if (value == -1)
    {
        print_scaled_result(1800000, FP);
        return;
    }
    if (value == 0)
    {
        print_scaled_result(900000, FP);
        return;
    }

    print_scaled_result(0, FP);
}

void calc_atan(int value)
{
    int z = value * FP;
    int radians = atan_fp_core(z);
    int degrees = rad_to_deg_fp(radians);
    print_scaled_result(degrees, FP);
}

void calc_actg(int value)
{
    if (value == 0)
    {
        print_scaled_result(900000, FP);
        return;
    }

    int z = value * FP;
    int atan_deg = rad_to_deg_fp(atan_fp_core(z));
    int result = 900000 - atan_deg;
    print_scaled_result(result, FP);
}

void calc_factorial(int n)
{
    if (n < 0)
    {
        ksys_print("Error: factorial is undefined for negative numbers.\n");
        return;
    }

    if (n > 12)
    {
        ksys_print("Error: factorial overflows int32 for n > 12.\n");
        return;
    }

    int result = 1;
    for (int i = 2; i <= n; ++i)
        result *= i;

    print_result(result);
}

void calc_tetration(int base, int height)
{
    if (height < 0)
    {
        ksys_print("Error: tetration height must be >= 0.\n");
        return;
    }

    if (height == 0)
    {
        print_result(1);
        return;
    }

    if (base == 0)
    {
        print_result((height % 2 == 0) ? 1 : 0);
        return;
    }

    if (base < 0)
    {
        ksys_print("Error: negative base is not supported for tetration in integer mode.\n");
        return;
    }

    int result = base;
    for (int h = 1; h < height; ++h)
    {
        if (result < 0)
        {
            ksys_print("Error: tetration intermediate exponent is invalid.\n");
            return;
        }

        if (result > 31)
        {
            ksys_print("Error: tetration grows too fast for int32 evaluation.\n");
            return;
        }

        int power = 1;
        for (int i = 0; i < result; ++i)
        {
            int tmp;
            if (!checked_mul_int(power, base, tmp))
            {
                ksys_print("Error: tetration overflow.\n");
                return;
            }
            power = tmp;
        }
        result = power;
    }

    print_result(result);
}

void calc_fibonacci(int n)
{
    if (n < 0)
    {
        ksys_print("Error: fibonacci index must be >= 0.\n");
        return;
    }

    if (n > 46)
    {
        ksys_print("Error: fibonacci overflows int32 for n > 46.\n");
        return;
    }

    int a = 0;
    int b = 1;

    if (n == 0)
    {
        ksys_print("Result: 0\n");
        return;
    }

    for (int i = 1; i < n; ++i)
    {
        int next = a + b;
        a = b;
        b = next;
    }

    print_result(b);
}