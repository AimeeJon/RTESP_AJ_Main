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
    // Finds the number of values in the ring buffer
    int channelOne = dataStore.size();

    // Sets the current plot value to 0
    double inVal = 0;

    // Runs a for loop for each value within the ring buffer
    for( int number=0; number<channelOne; ++number )
    {
        // Gets the first value from the ring buffer
        inVal = dataStore.front();

        // Converts the value to volts
        inVal = inVal * volt / sbit / gainOne;

        // Converts the value to resistance (k ohms)
        rTwo = rOne * ( inVal + 0.25 ) / ( vSupply - inVal - 0.25 ) / 1000;

        // Removes the value from the ring buffer
        dataStore.pop();

        // Adds the value to the curve
        memmove( yDataTop, yDataTop+1, (plotDataSizeTop-1) * sizeof(double) );
        yDataTop[plotDataSizeTop-1] = rTwo;
    }

    // Updates the plot with new values
    curveTop->setSamples(xDataTop, yDataTop, plotDataSizeTop);
    plotTop->replot();

    // Finds the number of values in the ring buffer
    int channelTwo = channelStore.size();

    // Sets the current plot value to 0
    double twoVal = 0;

    // Runs a for loop for each value within the ring buffer
    for( int numberTwo=0; numberTwo<channelTwo; ++numberTwo )
    {

        // Gets the first value from the ring buffer
        twoVal = channelStore.front();

        // Converts the value to volts
        twoVal = twoVal * volt / sbit / gainTwo;

        // Converts the value to pressure (mmHg)
        twoVal = twoVal * 1.29 * 7.52 / 1000;

        // Removes the value from the ring buffer
        channelStore.pop();

        // Adds the value to the curve
        memmove( yDataBottom, yDataBottom+1, (plotDataSizeBottom-1) * sizeof(double) );
        yDataBottom[plotDataSizeBottom-1] = twoVal;
    }

    // Updates the plot with new values
    curveBottom->setSamples(xDataBottom, yDataBottom, plotDataSizeBottom);
    plotBottom->replot();

    // Updates the number of values in each ring buffer
    channelOne = dataStore.size();
    channelTwo = channelStore.size();
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
    // Toggles the channel currently being measured and plotted
    if(toggleOne == 1)
    {
        toggleOne = 0;
        toggleTwo = 1;
    }
    else
    {
        toggleOne = 1;
        toggleTwo = 0;
    }
}
