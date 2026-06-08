#include <stdio.h>
#include <math.h>
#include "calc.h"
#include "logger.h"

int main(void) {
    float a = 12.5f, b = 2.5f, c = 0.0f;
    char log_buf[128];

    log_write("Application started.");

    // Addition
    float res_add = calc_add(a, b);
    snprintf(log_buf, sizeof(log_buf), "Add: %f + %f = %f", a, b, res_add);
    log_write(log_buf);

    // Division Valid
    float res_div = calc_div(a, b);
    snprintf(log_buf, sizeof(log_buf), "Div: %f / %f = %f", a, b, res_div);
    log_write(log_buf);

    // Division by zero
    float res_zero = calc_div(a, c);
    if (isnan(res_zero)) {
        log_error("Division by zero encountered!");
    }

    log_write("Application finished smoothly.");

    // Print summary to terminal
    printf("Execution complete. Checks performed:\n");
    printf("  - %f + %f = %f\n", a, b, res_add);
    printf("  - %f / %f = %f\n", a, b, res_div);
    printf("  - %f / %f = %s\n", a, c, isnan(res_zero) ? "NAN (Logged Error)" : "Success");
    printf("\nCheck 'app.log' for detailed output.\n");

    return 0;
}