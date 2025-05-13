#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define INPUT_SIZE 2
#define HIDDEN_SIZE 1024
#define OUTPUT_SIZE 1
#define ALPHA 0.1
#define EPOCHS 10000

// XOR input and targets
double inputs[4][2] = {
    {0, 0},
    {0, 1},
    {1, 0},
    {1, 1}
};
double targets[4] = {0, 1, 1, 0};

// Activation function
double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

// Random init
double rnd() {
    return (rand() % 2000) / 1000.0 - 1.0; // -1.0 to 1.0
}

// Shared error over neighbors
double shared_error(int i, double *errors, int size) {
    double sum = 0.0;
    int count = 0;
    for (int j = i - 1; j <= i + 1; j++) {
        if (j >= 0 && j < size) {
            sum += errors[j];
            count++;
        }
    }
    return sum / count;
}

int main() {
    srand(time(NULL));

    // weights: input -> hidden
    double w1[HIDDEN_SIZE][INPUT_SIZE];
    for (int i = 0; i < HIDDEN_SIZE; i++)
        for (int j = 0; j < INPUT_SIZE; j++)
            w1[i][j] = rnd();

    // weights: hidden -> output
    double w2[OUTPUT_SIZE][HIDDEN_SIZE];
    for (int i = 0; i < OUTPUT_SIZE; i++)
        for (int j = 0; j < HIDDEN_SIZE; j++)
            w2[i][j] = rnd();

    // training
    for (int epoch = 0; epoch < EPOCHS; epoch++) {
        for (int n = 0; n < 4; n++) {
            // forward pass
            double hidden[HIDDEN_SIZE];
            for (int i = 0; i < HIDDEN_SIZE; i++) {
                hidden[i] = 0;
                for (int j = 0; j < INPUT_SIZE; j++)
                    hidden[i] += w1[i][j] * inputs[n][j];
                hidden[i] = sigmoid(hidden[i]);
            }

            double output = 0;
            for (int j = 0; j < HIDDEN_SIZE; j++)
                output += w2[0][j] * hidden[j];
            output = sigmoid(output);

            double error_out = targets[n] - output;

            // calculate hidden errors
            double hidden_errors[HIDDEN_SIZE];
            for (int i = 0; i < HIDDEN_SIZE; i++)
                hidden_errors[i] = error_out * w2[0][i];

            // update w2 (hidden→output)
            for (int j = 0; j < HIDDEN_SIZE; j++)
                w2[0][j] += ALPHA * error_out * hidden[j];

            // update w1 (input→hidden) with shared errors
            for (int i = 0; i < HIDDEN_SIZE; i++) {
                double shared = shared_error(i, hidden_errors, HIDDEN_SIZE);
                for (int j = 0; j < INPUT_SIZE; j++)
                    w1[i][j] += ALPHA * shared * inputs[n][j];
            }
        }
    }

    // test
    printf("Result after training:\n");
    for (int n = 0; n < 4; n++) {
        double hidden[HIDDEN_SIZE];
        for (int i = 0; i < HIDDEN_SIZE; i++) {
            hidden[i] = 0;
            for (int j = 0; j < INPUT_SIZE; j++)
                hidden[i] += w1[i][j] * inputs[n][j];
            hidden[i] = sigmoid(hidden[i]);
        }

        double output = 0;
        for (int j = 0; j < HIDDEN_SIZE; j++)
            output += w2[0][j] * hidden[j];
        output = sigmoid(output);

        printf("in: %.0f %.0f -> out: %.3f\n", inputs[n][0], inputs[n][1], output);
    }

    return 0;
}
