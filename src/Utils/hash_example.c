#include "hash.h"

int main () {
	FILE* fp = fopen("dnsrelay.txt" , "r");
	
	hash* mapped = (hash*)malloc(sizeof(hash));
	init_hash(mapped);
	
	char buffer[512];
	while (fgets(buffer , sizeof buffer , fp) && !feof(fp)) {

	/* NOTE : In Win , the line feed signal is  '\n' , 
	 *		  However in Linux , it is '\r\n'
	 */
	
	/* remove blank line */
	#ifdef _WIN32
		if(buffer[0] =='\n') continue;
	#else
		if(buffer[0] =='\r') continue;
	#endif

		char* val = strtok(buffer , " " );

	/* split one line string to k-v */
	#ifdef _WIN32
		char* key = strtok(NULL   , "\n");
	#else
		char* key = strtok(NULL   , "\r");
					strtok(NULL   , "\n");
	#endif
		
		//puts(key); puts(val);
		
		if (insert_hash(mapped , key , val , STRMEM(val)) == FAILURE) {
			//memset(buffer , 0 , sizeof buffer);
			char concat[512];
			memset(concat , 0 , sizeof concat);
			
			query_hash(mapped , key , concat , sizeof concat);
		
			//strcat(buffer , val);
			printf("Duplicated %s : %s + %s\n" ,key ,  concat , val);
		}
	}
	count_hash(mapped);
	
	memset(buffer , 0 , sizeof buffer);
	if (query_hash(mapped , "www.bupt.cn" , buffer , sizeof buffer) == SUCCUSS)
		printf("ans is : %s\n" , buffer);
	
	if (remove_hash(mapped , "008.cn") == SUCCUSS)
		printf("REMOVE SUCCESSFULLY\n");
	
	if (query_hash(mapped , "www.bupt.cn" , buffer , sizeof buffer) == SUCCUSS)
		printf("ANS : %s\n" , buffer);
	else printf("FAILURE\n");
	
	
	char ips[] = "0.0.0.0/1.1.1.1";
	modify_hash(mapped , "www.bupt.cn" , ips , STRMEM(ips));
	if (query_hash(mapped , "www.bupt.cn" , buffer , sizeof buffer) == SUCCUSS)
		printf("ANS is : %s\n" , buffer);	
	
	free_hash(mapped);
	
	fclose(fp);
	return 0;
}
