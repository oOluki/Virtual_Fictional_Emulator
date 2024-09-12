#include <stdlib.h>
#include <stdio.h>

void show_usage(){

printf(
"[HELP] Usage: ./ve <path_to_executable>\n"
);

}



int main(int argc, char** argv){

if(argc < 2){
show_usage();
printf("[ERROR] Expected At Least One Argument, Got NONE Instead\n");
}




return 0;
}

