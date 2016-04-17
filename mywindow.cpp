#include "Iir.h"
#include "mywindow.h"
#include "adcreader.h"

#include <cmath>
#include <queue>
#include <QCoreApplication>

Window::Window() : gain(5), count(0)
{
		int indexTop = 0;
		int indexBottom = 0;

                zoomA = new QPushButton("Toggle Channel",this);

		connect( zoomA, SIGNAL (clicked()),this, SLOT (toggleChannel()) );
		
                zoomB = new QPushButton("Button 1");
		
                start = new QPushButton("Button 2");
		
                stop = new QPushButton("Button 3");
		
                print = new QPushButton("Button 4");
		
		curveTop = new QwtPlotCurve;
		plotTop = new QwtPlot;
		
		curveBottom = new QwtPlotCurve;
		plotBottom = new QwtPlot;
		
		for( indexTop=0; indexTop<plotDataSizeTop; ++indexTop )
		{
			xDataTop[indexTop] = indexTop;
			//yDataTop[indexTop] = gain * sin( M_PI * indexTop/50);			
			yDataTop[indexTop] = 0;
		}
		
		for( indexBottom=0; indexBottom<plotDataSizeBottom; ++indexBottom )
		{
			xDataBottom[indexBottom] = indexBottom;
			//yDataBottom[indexBottom] = gain * sin( M_PI * indexBottom/50);			
			yDataBottom[indexBottom] = 0;
		}
		
		curveTop->setSamples(xDataTop, yDataTop, plotDataSizeTop);
		curveTop->attach(plotTop);
		
		curveBottom->setSamples(xDataBottom, yDataBottom, plotDataSizeBottom);
		curveBottom->attach(plotBottom);


		plotTop->setTitle( "Skin Resistance" );
		plotTop->setAxisTitle( QwtPlot::xBottom, "Time (Samples)" );
		plotTop->setAxisTitle( QwtPlot::yLeft, "Resistance (k Ohms)" );
		plotTop->setAxisScale(QwtPlot::xBottom,0, 10000, 0);

		plotBottom->setTitle( "Blood Pressure" );
		plotBottom->setAxisTitle( QwtPlot::xBottom, "Time (Samples)" );
		plotBottom->setAxisTitle( QwtPlot::yLeft, "Pressure (mmHg)" );
		plotBottom->setAxisScale(QwtPlot::xBottom,0, 10000, 0);
		
		plotTop->replot();
		plotTop->show();
		
		plotBottom->replot();
		plotBottom->show();
		
		vLayoutLeft = new QVBoxLayout;
		vLayoutLeft->addWidget(zoomA);
		vLayoutLeft->addWidget(zoomB);
		vLayoutLeft->addWidget(start);
		vLayoutLeft->addWidget(stop);
		vLayoutLeft->addWidget(print);
		
		vLayoutRight = new QVBoxLayout;
		vLayoutRight->addWidget(plotTop);
		vLayoutRight->addWidget(plotBottom);
		
		hLayout = new QHBoxLayout;
		hLayout->addLayout(vLayoutLeft);
		hLayout->addLayout(vLayoutRight);
		
		setLayout(hLayout);

		//dataStore.push(20);

                adcreader = new ADCreader(&dataStore,&channelStore, &toggleOne,&toggleTwo);
                adcreader->start();
		


}


Window::~Window() {
	
        adcreader->quit();
	
        adcreader->wait();
        delete adcreader;
}


void Window::timerEvent( QTimerEvent *)
{
	int channelOne = dataStore.size();
	double inVal = 0;
	//double twoVal = 0;
	int test = 0;
	//int empty = dataStore.empty();
	
	//int channelTwo = channelStore.size();
	//fprintf(stderr,"contain = %d  \r",contain);

	for( int number=0; number<channelOne; ++number )
	//fprintf(stderr,"test = %d \r",empty);
	//while(channelOne > 0)
	{
  	  //printf("1\n");
	  //while(channelOne > 0)
	  //{
	    inVal = dataStore.front();
	    //stepOne = volt/gainOne;
	    //stepTwo = stepOne/sbit;
	    //inVal = inVal * 3300 / 65536 / 8;
	    inVal = inVal * volt / sbit / gainOne;

	    rTwo = rOne * ( inVal + 0.25 ) / ( vSupply - inVal - 0.25 ) / 1000;
	    //fprintf(stderr,"test = %d          \r",stepOne);

	    dataStore.pop();
	    //channelOne = dataStore.size();

	    //printf("test1\n");

	    memmove( yDataTop, yDataTop+1, (plotDataSizeTop-1) * sizeof(double) );
	    yDataTop[plotDataSizeTop-1] = rTwo;
	  }

          curveTop->setSamples(xDataTop, yDataTop, plotDataSizeTop);
	  plotTop->replot();
	  

	  int channelTwo = channelStore.size();
	  double twoVal = 0;

	  //printf("2\n");
	  for( int numberTwo=0; numberTwo<channelTwo; ++numberTwo )
	  {
	    twoVal = channelStore.front();
	    //test = twoVal * 3300 / 65536 / 32;
	    twoVal = twoVal * volt / sbit / gainTwo;

	    twoVal = twoVal * 1.29 * 7.52 / 1000;
	    //fprintf(stderr,"twoVal = %d\r", test);

	    channelStore.pop();
	    //channelTwo = channelStore.size();
	    //printf("test2\n");

	    memmove( yDataBottom, yDataBottom+1, (plotDataSizeBottom-1) * sizeof(double) );
	    yDataBottom[plotDataSizeBottom-1] = twoVal;
	  }

	  curveBottom->setSamples(xDataBottom, yDataBottom, plotDataSizeBottom);
	  plotBottom->replot();
	  
	  //fprintf(stderr,"inVal = %d\r",inVal);
	  //printf("3\n");
	  //printf("4\n");
	  //printf("5\n");
	  //double inVal = gain * sin( M_PI * count/50.0 );
	  //++count;
	  //printf("6\n");
	  // add the new input to the plot
	  //memmove( yDataTop, yDataTop+1, (plotDataSizeTop-1) * sizeof(double) );
	  //yDataTop[plotDataSizeTop-1] = inVal;
	  //curveTop->setSamples(xDataTop, yDataTop, plotDataSizeTop);
	  //plotTop->replot();

	  //printf("7\n");
	  //memmove( yDataBottom, yDataBottom+1, (plotDataSizeBottom-1) * sizeof(double) );
	  //yDataBottom[plotDataSizeBottom-1] = twoVal;
	  //curveBottom->setSamples(xDataBottom, yDataBottom, plotDataSizeBottom);
	  //plotBottom->replot();
	  //printf("8\n");
	  //dataStore.pop();
	  //empty = dataStore.size();
	  
	  //curveTop->setSamples(xDataTop, yDataTop, plotDataSizeTop);
	  //plotTop->replot();

	  channelOne = dataStore.size();
	  channelTwo = channelStore.size();
	//}

	//plotTop->replot();
	//plotBottom->replot();
}




void Window::setGain(double gain)
{
	this->gain = gain;
}


//void Window::startThread(void)
//{
//    testFunction();
//}

void Window::toggleChannel()
{

	printf("test\n");
	//fprintf(stderr,"Test = %d /r",toggleOne);

	if(toggleOne == 1)
	{
	  toggleOne = 0;
	  toggleTwo = 1;
//	  zoomA->resize(100,50);
	}
	else
	{
	  toggleOne = 1;
	  toggleTwo = 0;
//	  zoomA->resize(50,100);
	}
}
