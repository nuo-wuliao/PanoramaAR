#include <iostream>
#include "panoar.h"

using namespace std;

int main()
{
	cout << "panoAR start!" << endl;

	PanoAR *pAR = new PanoAR(2048, 1024);
	pAR->run();

	return 0;
}