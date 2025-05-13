#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX 4
#define EPOCHS 1
#define POS 0.2
#define NEG 0.1
#define ALPHA 0.1

int inputs[4][2] = {
	{0, 0},
	{0, 1},
	{1, 0},
	{1, 1}
};

int targets[4] = {0, 0, 0, 1};


void print_matrix(int rows, int cols, double *matrix, const char* label) {
	printf("%s:\n", label);                                                                      for (int i = 0; i < rows; i++) {                                                                 	for (int j = 0; j < cols; j++) {                                                     		printf("%4f ", (float)matrix[i * cols + j]);
	      }
	printf("\n");
	}                                                                                            printf("\n");                                                                            } 


void init(double **input, double **weight, double **out, int size){
	*weight = (double *)malloc(sizeof(double) * size);
	if(*weight == NULL){
		perror("weight malloc faild\n");	
		exit(1);
	}
	*input = (double *)malloc(sizeof(double) * size);
	if(*input == NULL){
		perror("weight malloc faild\n");	
		exit(1);
	}
	*out = (double *)malloc(sizeof(double) * size / 2);
	if(*out == NULL){
		perror("weight malloc faild\n");	
		exit(1);
	}
	for(int j = 0; j <= size; j++){
			if(((j + 1) % 2) == 0){
				(*input)[j] = -1;	
			}else{
				(*input)[j] = 1;
			}	
	}
	double *temp = (double *)realloc(*weight, sizeof(double) * size);	
	if(temp == NULL){
		perror("realloc faild");	
		exit(1);
	}
	*weight = temp;
	for(int i = 0; i <= size; i++){
	//delによるculc
		if(((i) % 2) == 0){
			//pos

			(*weight)[i] = POS;
			//(*weight)[i] = rnd();
		}else{
			//neg
			(*weight)[i] = NEG;
		}
	}

}

double rnd(){
	srand(time(NULL));
	return ((rand() % 1000) / 1000.0);
}

//frontに値を回すときのどうしたら学習するか、隣のoutにも値を渡して誤差を共有してXORを学習させる

//hebbian w[i] = ALPHA * input[i] * output[i];
void update(double **weight, double *input, double *error, int size){
	for(int i = 0; i < size; i++){
		(*weight)[i] += ALPHA * error[i] * input[i];	
	}	

}

void culc(double **input, double **weight, double **out, int size){
	double ans = 0;
	int j = 0;
	for(int i = 0; i < (size / 2); i++){
		ans = (*input)[2 * i] * (*weight)[2 * i] + (*input)[2 * i + 1] * (*weight)[2 * i + 1];	

		(*out)[i] = fmod(ans, (double)MAX);

	}
}

int main(){
	int e = 1;
	//最大は2^27乗まで２８からはプロセス殺される
	//134217728
	for(int i = 0; i < 2; i++){
		printf("%d\n", e *=2);	
	}
	int size = MAX;
	int n = MAX;
	int NMAX = 1;
	while(n != 1){
		n = n / 2;	
		NMAX++;
	}
	double *weight;
	double *input;
	double *out;
	double ans;
	double error;

	init(&input, &weight, &out, size);
	//print_matrix(size, 1, input, "input");

	sina_c(&weight, size, 0);
	for(int i = 0; i < NMAX; i++){
		for()	
		culc(&input,&weight,&out,size);
		memcpy(input, out, sizeof(double) * size / 2);
		input = (double *)realloc(input, sizeof(double) * size);	
		out = (double *)realloc(out, sizeof(double) * size / 2);	
		error = target[i] - out;
		update(&weight, size, )
		//print_matrix(size / 2, 1, input, "input");
		print_matrix(size / 2, 1, out, "out");
		print_matrix(size / 2, 1, weight, "weight");
		size = size / 2;
	}
	print_matrix(1, 1, out, "out");
	free(input);
	free(weight);
	free(out);

	return 0;
}
