#include <bits/stdc++.h>
using namespace std ; 
int main(int argc, char const *argv[])
{
	FILE * fp ; 
	fp = fopen("secondPingText.txt" , "r") ; 
	char ch = fgetc(fp);
	int i = 0 ;
	string str ;  
	std::vector<float> time; 
	while(ch != EOF){
		i++ ;
		str = "";
		while(ch != '\n'){
			str = str + ch ; 
			ch = fgetc(fp);
		}
		//cout<<str<<endl;
		int index = str.size()-1 ;
		for ( index = str.size() -1 ; index >= 0  ; --index){
			if (str[index] == '=')
			{
				break; 
			}
		}
		string str2 = ""; 
		for (int k = index+1; k < str.size() ; ++k)
		{
			if (str[k] == ' '){
				break ;
			}
			str2 = str2 + str[k];
		}
		//cout<<str2<<endl;
		if (i >= 2 && i <= 1000){
			float temp = ::atof(str2.c_str());

			time.push_back(temp);
		}
		//printf("%c", ch );
		ch = fgetc(fp);
	} 
	fclose(fp);
	sort(time.begin() , time.end());
	/*for (int i = 0; i < time.size(); ++i)
	{
		cout<<time[i]<<endl;
	}*/
	cout<<time.size()<<endl;
	cout<<(time[500] + time[501])/ 2<<endl;
	fp = fopen("Result2.txt" , "w") ; 
	for (int i = 0; i < time.size(); ++i){
		char str[256];
 		int x = 0 ;
        sprintf( str, "%f", time[i]);
        fprintf( fp ,  str , x+ 1 );
        fprintf(fp, "\n" );
	}
	fclose(fp);
	return 0;
}