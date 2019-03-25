
#include "Printer.h"


std::vector<Printer *> PrinterList;
std::vector<Printer *> PrinterUpdateThreadList;
Printer *Selected = 0;
int SortOrder = 1;

int PrinterColumns[10];
bool RefreshingPrinters = false;

int MinStatusLength = 0;
int MaxStatusLength = 50;
int NetworkTimeout = 2;


void InitPrinters()
{
	DestroyPrinters();
	
	//PrinterListGuard.lock();
	
	std::ifstream file("Printers.txt");
	std::string line;
	
	while(std::getline(file, line))
	{
		if (line.size())
		{
			PrinterList.push_back(new Printer(line));
			PrinterUpdateThreadList.push_back(PrinterList.back());
		}
	}
	
	file.close();
	
	//mutexes created after adding printers to list to handle how std::vector copying and deleting Printer object issues
	/*for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		PrinterList[i].Mutex = new std::mutex;
	}*/
	//PrinterListGuard.unlock();
	
	SortPrinters();
}

void DestroyPrinters()
{
	//PrinterListGuard.lock();
	
	for (int i = 0; i < (int)PrinterList.size(); i++)
	{
		if (PrinterList[i]) { delete PrinterList[i]; }
	}
	
	PrinterList.clear();
	PrinterUpdateThreadList.clear();
	
	//PrinterListGuard.unlock();
}


std::string Search(std::string str, std::string delim, int offset, int *i)
{
	std::string found;
	
	int s = offset, d = 0;
	bool any = false;
	bool copy = false;
	while (s < (int)str.size() && d < (int)delim.size())
	{
		if (delim[d] == '*')
		{
			any = true;
			d++;
		}
		else if (delim[d] == '@')
		{
			copy = true;
			d++;
		}
		else if (any)
		{
			if (str[s] == delim[d])
			{
				any = false;
			}
			else
			{
				s++;	
			}
		}
		else if (copy)
		{
			if (str[s] == delim[d])
			{
				copy = false;
			}
			else
			{
				found += str[s];
				s++;
			}
		}
		else if (str[s] == delim[d])
		{
			s++;
			d++;
		}
		else
		{
			if (d == 0) { s++; }
			else { d = 0; }
		}
	}
	
	if (d < (int)delim.size()) { return "-1"; }
	if (i) { *i = s; }
	return found; 
}

void Replace(std::string &str, std::string find, std::string replace)
{
	int s = 0, f = 0, r = -1;
	while (s < (int)str.size())
	{
		if (str[s] == find[f])
		{
			if (r<0) { r = s; }
			s++;
			f++;
		}
		else
		{
			if (f == 0) { s++; }
			else { f = 0; }
			r = -1;
		}
		
		if (f >= (int)find.size())
		{
			str.replace(r, s-r, replace);
			s -= (s-r);
			f = 0;
			r = -1;
		}
	}
}

int First(std::string str, std::string first, std::string second, int offset, int *i)
{	
	size_t s1 = str.find(first, offset);
	size_t s2 = str.find(second, offset);
	
	if (s1 == std::string::npos && s2 == std::string::npos) { return -1; }
	
	if (i) { *i = MIN(s1, s2); }
	return (s1 <= s2 ? 1 : 0);
}

std::string MinSize(std::string str, int size, char fill)
{
	return str + std::string(MAX(size-(int)str.size(), 0), fill);
}

std::string MaxSize(std::string str, int size)
{
	return str.substr(0, MIN(size, (int)str.size()));
}
