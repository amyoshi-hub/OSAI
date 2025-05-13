#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NMAX 6 // 列数
#define MAX 16  // 行数

double node[MAX][NMAX];
double o_node[MAX][NMAX];
double weight[MAX][NMAX];
double del;

int inputs[4][2] = {
	{0, 0},
	{0, 1},
	{1, 0},
	{1, 1}
};

int targets[4] = {0, 1, 1, 0};

double rnd(){
	srand(time(NULL));
	return ((rand() % 1000) / 10.0);
}

void clear() {
    printf("\033[2J");
}

void print_matrix(int rows, int cols, double matrix[rows][cols], const char* label) {
    printf("%s:\n", label);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%4f ", (float)matrix[i][j]); // 各値を4文字幅で整列
        }
        printf("\n");
    }
    printf("\n");
}

void calc(double node[MAX][NMAX], double weight[MAX][NMAX], int loop){
	for(int i = 0; i <= NMAX; i++){
		for(int j = 0; j <= MAX; j++){
			//o_nodeを二層目のa11から数えていけばいい
			//基本は[j][i]単体だが周りの２つにも誤差を伝える
			double del = weight[j][loop] * node[j][loop] + weight[j + 1][loop] * node[j + 1][loop];
			del = abs(del);
			printf("del>%lf\n", del);
			//if(0.3 > del){
				o_node[j][i] = del;
				o_node[j - 1][i] += del;
				o_node[j + 1][i] += del;
			//}
			
		}
	}

}



int main() {
    clear();
    
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < NMAX; j++) {
            if ((i % 2) == 0)
                node[i][j] = 1;
            else
                node[i][j] = -1;
        }
    }

    //ここで上がポジティブと仲良しここで拮抗させると０ポジティブに対して大きくすれば出力はプラス
    //に入力の判定はできたということだろうか？
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < NMAX; j++) {
		if((i % 2) == 0){
			weight[i][j] = 2;
		}else{
			weight[i][j] = 1;	
		}
        }
    }
    int loop = 0;

    for(int i = 0; i < NMAX; i++){
    	calc(node, weight, loop);
	loop++;
    }

    print_matrix(MAX, NMAX, node, "Node Matrix");
    print_matrix(MAX, NMAX, weight, "Weight Matrix");
    print_matrix(MAX, NMAX, o_node, "out node");


    //printf("%lf", o_node[MAX/2][NMAX/2]);
    return 0;
}
