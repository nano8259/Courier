/*
 * macroflow-config.cc
 *
 *  Created on: Apr 11, 2017
 *      Author: tbc
 */


#include <string>
#include <cctype>
#include <fstream>
#include <cmath>
#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "ns3/internet-module.h"

using namespace std;
using namespace ns3;

double Str2Uint(string str)
{
	uint32_t ans = 0;
	uint32_t i=0;
	for(;i<str.length() && isdigit(str[i]);++i)
		ans = ans*10 + str[i]-'0';

	double ans2 = 0;
	double cnt = 1;
	if(str[i++]=='.'){
		for(;i<str.length() && isdigit(str[i]);++i){
			cnt*=0.1;
			ans2+= cnt*(str[i]-'0');
		}
	}

	return ans+ans2;
}

const double eps = 1e-6;

bool eq(double a,double b){
	return fabs(a-b)<eps;
}

bool ge(double a,double b){
	return eq(a,b) || a>b;
}

bool le(double a,double b){
	return eq(a,b) || a<b;
}

uint64_t dbl2int(double d){
	return (uint64_t)round(d*1e6);
}

bool isInt(double d){
	return eq(d,round(d));
}

int
main (int argc, char *argv[])
{
	if(argc-1 < 6 || argc-1 > 7){
		cout<<"Usage: "<<endl;
		cout<<"macroflow-config <template-ini-file> <ini-section> <ini-item> <min-uint-value> <max-uint-value> <step> [<suffix>]"<<endl;
		return 0;
	}

	string file = argv[1];
	if(file.substr(file.length()-4).compare(".ini")!=0){
		cout<<"config file should written in ini-format (*.ini)"<<endl;
		return 0;
	}
	string section= argv[2];
	ToLower(section);
	string item = argv[3];
	ToLower(item);
	double minValue = Str2Uint(argv[4]);
	double maxValue = Str2Uint(argv[5]);
	double step = Str2Uint(argv[6]);
	string suffix = "";
	if(argc-1==7) suffix = argv[7];

	boost::property_tree::ptree root;
	boost::property_tree::read_ini(file, root);
	string path = section + "." + item;
	if(root.get<string>(path,"").length()==0){
		cout<<"error: no such property:"<<path<<endl;
		return 0;
	}

	bool is_int = isInt(minValue) && isInt(maxValue) && isInt(step);

	// output ini file
	file = file.substr(0,file.length()-4); // remove ".ini"
	ofstream fout((file+".sh").c_str());
	fout<<"./waf build"<<endl;
	if(is_int)
		fout<<"for((i="<<minValue<<";i<="<<maxValue<<";i+="<<step<<")) do"<<endl;
	else
		fout<<"for((i="<<dbl2int(minValue)<<";i<="<<dbl2int(maxValue)<<";i+="<<dbl2int(step)<<")) do"<<endl;
	fout<<"{"<<endl;
	string out = file + "-" + item +"-$i";
	//fout<<"    ./waf --run \"macroflow-main "<<(out+".ini")<<" "<<(out+".log")<<"\" 2>"+out+".error"<<endl;
	fout<<"    ./waf --run \"macroflow-main "<<(out+".ini")<<" "<<("result/"+out+".log")<<"\" 2> result/"+out+".error"<<endl;
	fout<<"}&"<<endl;
	fout<<"done"<<endl;
	fout.close();

	// output sh file
	for(double i = minValue; le(i,maxValue); i+=step){
		if(isInt(i))
			i=round(i);
		else
			i=round(i*1e6)/1e6;


		string value = ToString(i)+suffix;
		string filename ="";

		if(is_int)
			filename = ToString(i)+suffix;
		else
			filename = ToString(dbl2int(i))+suffix;

		root.put<string>(path,value);
		out = file + "-" + item +"-"+filename;
		boost::property_tree::ini_parser::write_ini(out+".ini",root);
		//out = file + "-" + item +"-"+value;
	}
	cout<<"Done."<<endl<<endl;

	return 0;

}

