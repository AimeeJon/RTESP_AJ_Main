#ifndef MYWINDOW_H
#define MYWINDOW_H


#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_arrow_button.h>
#include <thread>
#include <queue>
#include <QPushButton>

#include <QBoxLayout>

#include "adcreader.h"

extern void testFunction(void);

class Window : public QWidget
{
	Q_OBJECT
	
public:
	Window();
	
	~Window();

	void timerEvent( QTimerEvent * );
	
public slots:
	void setGain(double gain);
	void toggleChannel();
	
private:
	QPushButton	*zoomA;
	QPushButton	*zoomB;
	QPushButton	*start;
	QPushButton	*stop;
	QPushButton	*print;
	QwtPlot			*plotTop;
	QwtPlot			*plotBottom;
	QwtPlotCurve	*curveTop;
	QwtPlotCurve	*curveBottom;
	
	QVBoxLayout	*vLayoutLeft;
	QVBoxLayout	*vLayoutRight;
	QHBoxLayout	*hLayout;
	
	static const int plotDataSizeBottom = 10000;
	static const int plotDataSizeTop = 10000;
	
	double xDataTop[plotDataSizeTop];
	double yDataTop[plotDataSizeTop];
	
	double xDataBottom[plotDataSizeBottom];
	double yDataBottom[plotDataSizeBottom];
	
	double gain;
	int count;
	int contain;
	int number;
	double inVal;
	double twoVal;
	int channelOne;
	int channelTwo;
	int toggleOne = 1;
	int toggleTwo = 0;

	int gainOne = 8;
	int gainTwo = 64;
	int sbit = 65536;
	int volt = 3.3;

	int rOne = 1000000;
	double vSupply = 0.5;
	double rTwo;

	std::thread readPin;
	bool collect;
	std::queue <int> dataStore;
	std::queue <int> channelStore;
	bool running;

	ADCreader *adcreader;
	
};

#endif
