#ifndef ADCREADER
#define ADCREADER

#include <bcm2835.h>
#include <QThread>
#include <queue>

class ADCreader : public QThread
{
public:
	ADCreader(std::queue <int>* q,std::queue <int>* qu,int* to, int* tt) {running = 0; threadQueue = q; channelQueue = qu; selectOne = to; selectTwo = tt;};
	void quit();
	void run();
private:
	bool running;
	std::queue <int>* threadQueue;
	std::queue <int>* channelQueue;
	int* selectOne;
	int* selectTwo;
};

#endif
