#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int cluc_sum(char *data){
	int sum;
	sum += data[1];
	printf("file:%s sum:%d\n", data, sum);
	return sum;
}

int file_get(char* filename){
	int value[10];
	char buf[20];
	char *dir="data/";
	char name[30];
	int sum = 0;
	sprintf(name, "%s%s", dir, filename);

	FILE *fp = fopen(name, "r");
	if(fp == NULL){
		perror("file open error\n");
		exit(1);
	}
	//recv
	while(fgets(buf, sizeof(buf), fp) != NULL){
		sum = cluc_sum(buf);
	}
	return sum;
}

#ifdef DEBUG
int main(){
	int sum1 = file_get("data1.txt");
	int sum2 = file_get("data2.txt");
	int sum3 = file_get("data3.txt");

}

#endif
