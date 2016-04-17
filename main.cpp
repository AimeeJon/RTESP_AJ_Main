
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <mywindow.h>

#include <thread>
#include <queue>
#include <QApplication>

#include "gz_clk.h"
#include "gpio-sysfs.h"


//void testFunction()
//{
//
//    while (true) {
//        qDebug() << "Tick";
//        sleep(1);
//    }
//}



int main(int argc, char *argv[])
{

        //if (argc == 12345)
        //{
        //    readReg(0);
        //}

	QApplication app(argc, argv);

	// create the window
	Window window;
	window.showMaximized();

        //window.startThread();

	// call the window.timerEvent function every 40 ms
        window.startTimer(40);

	// execute the application
	return app.exec();
}
