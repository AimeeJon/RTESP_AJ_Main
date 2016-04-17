#include "adcreader.h"
#include <QDebug>

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
#include "bcm2835.h"

static void pabort(const char *s)
{
        perror(s);
        abort();
}

static const char *device = "/dev/spidev0.0";
static uint8_t mode = SPI_CPHA | SPI_CPOL;;
static uint8_t bits = 8;
static int drdy_GPIO = 22;

static void writeReset(int fd)
{
  int ret;
  uint8_t tx1[5] = {0xff,0xff,0xff,0xff,0xff};
  uint8_t rx1[5] = {0};
  struct spi_ioc_transfer tr;

  memset(&tr,0,sizeof(struct spi_ioc_transfer));
  tr.tx_buf = (unsigned long)tx1;
  tr.rx_buf = (unsigned long)rx1;
  tr.len = sizeof(tx1);

  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  if (ret < 1) {
    printf("\nerr=%d when trying to reset. \n",ret);
    pabort("Can't send spi message");
  }
}

static void writeReg(int fd, uint8_t v)
{
  int ret;
  uint8_t tx1[1];
  tx1[0] = v;
  uint8_t rx1[1] = {0};
  struct spi_ioc_transfer tr;

  memset(&tr,0,sizeof(struct spi_ioc_transfer));
  tr.tx_buf = (unsigned long)tx1;
  tr.rx_buf = (unsigned long)rx1;
  tr.len = sizeof(tx1);

  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  if (ret < 1)
    pabort("can't send spi message");
}

static uint8_t readReg(int fd)
{
        int ret;
        uint8_t tx1[1];
        tx1[0] = 0;
        uint8_t rx1[1] = {0};
        struct spi_ioc_transfer tr;

        memset(&tr,0,sizeof(struct spi_ioc_transfer));
        tr.tx_buf = (unsigned long)tx1;
        tr.rx_buf = (unsigned long)rx1;
        tr.len = sizeof(tx1);

        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if (ret < 1)
          pabort("can't send spi message");

        return rx1[0];
}

static int readData(int fd)
{
        int ret;
        uint8_t tx1[2] = {0,0};
        uint8_t rx1[2] = {0,0};
        struct spi_ioc_transfer tr;

        memset(&tr,0,sizeof(struct spi_ioc_transfer));
        tr.tx_buf = (unsigned long)tx1;
        tr.rx_buf = (unsigned long)rx1;
        tr.len = sizeof(tx1);

        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if (ret < 1)
          {
          printf("\n can't send spi message, ret = %d\n",ret);
          exit(1);
          }

        return (rx1[0]<<8)|(rx1[1]);
}







void ADCreader::run()
{
        int ret = 0;
        int fd;
        int sysfs_fd;

        int no_tty = !isatty( fileno(stdout) );

        fd = open(device, O_RDWR);
        if (fd < 0)
                pabort("can't open device");

        /*
         * spi mode
         */
        ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
        if (ret == -1)
                pabort("can't set spi mode");

        ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
        if (ret == -1)
                pabort("can't get spi mode");

        /*
         * bits per word
         */
        ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
        if (ret == -1)
                pabort("can't set bits per word");

        ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
        if (ret == -1)
                pabort("can't get bits per word");

        fprintf(stderr, "spi mode: %d\n", mode);
        fprintf(stderr, "bits per word: %d\n", bits);

        // enable master clock for the AD
        // divisor results in roughly 4.9MHz
        // this also inits the general purpose IO
        gz_clock_ena(GZ_CLK_5MHz,5);

        // enables sysfs entry for the GPIO pin
        gpio_export(drdy_GPIO);
        // set to input
        gpio_set_dir(drdy_GPIO,0);
        // set interrupt detection to falling edge
        gpio_set_edge(drdy_GPIO,"falling");
        // get a file descriptor for the GPIO pin
        sysfs_fd = gpio_fd_open(drdy_GPIO);


	//while(1) {
        // resets the AD7705 so that it expects a write to the communication register
        //printf("sending reset\n");
        writeReset(fd);

        // tell the AD7705 that the next write will be to the clock register
        writeReg(fd,0x20);
        // write 00001100 : CLOCKDIV=1,CLK=1,expects 4.9152MHz input clock
        writeReg(fd,0x0C);

        // tell the AD7705 that the next write will be the setup register
        writeReg(fd,0x10);
        // intiates a self calibration and then after that starts converting
        writeReg(fd,0x58);

	//writeReg(fd,0x01);
        // we read data in an endless loop and display it
        // this needs to run in a thread ideally
        while (1) {

	  writeReg(fd,0x10);
	  writeReg(fd,0x58);

	  while(*selectOne == 1)
	  {
		//printf("1\n");
	  	writeReg(fd,0x38);
          	// let's wait for data for max one second
          	ret = gpio_poll(sysfs_fd,1000);
          	if (ret<1) {
            	fprintf(stderr,"Poll error %d %d\n",ret, sysfs_fd);
          	}
	  	//printf("2\n");
          	// tell the AD7705 to read the data register (16 bits)
          	//writeReg(fd,0x38);
	  	//sleep(0.1);
	  	//printf("3\n");
          	// read the data register by performing two 8 bit reads
          	int value = readData(fd)-0x8000;
          	      //fprintf(stderr,"data = %d       \r",value);
	  	//printf("4\n");
	  	if (threadQueue->size() < 1000) {
	  		threadQueue->push(value);
	  	}
	  	//printf("5\n");
	  	//threadQueue->push(value);
          	int test = threadQueue->size();
	  	//fprintf(stderr,"number = %d     \r",test);
                // if stdout is redirected to a file or pipe, output the data
                	if( no_tty )
               		{
                        	printf("%d\n", value);
                        	fflush(stdout);
                	}
	  	//printf("6\n");

	  	//writeReg(fd,0x38);

	  	//writeReset(fd);
	  	//writeReg(fd,0x21);
	  	//writeReg(fd,0x0C);
	  	//writeReg(fd,0x11);
	  	//writeReg(fd,0x40);
	  }

	  writeReg(fd,0x10);
	  writeReg(fd,0x78);

	  while(*selectTwo == 1)
	  {

	  	writeReg(fd,0x39);

	  	//sleep(100);
	  	//printf("7\n");
	  	ret = gpio_poll(sysfs_fd,1000);
	  	if (ret<1) {
	    	fprintf(stderr,"Poll error %d %d\n",ret, sysfs_fd);
	  	}

	  	//writeReg(fd,0x39);
	  	//sleep(0.1);
	  	//printf("8\n");
	  	int valueTwo = readData(fd)-0x8000;
	  		//fprintf(stderr,"data = %d         \r", valueTwo);
	  	//printf("9\n");
	  	if (channelQueue->size() < 1000) {
	  		channelQueue->push(valueTwo);
	  	}
	  	//printf("10\n");
	  		if ( no_tty )
	  		{
	  			printf("%d\n", valueTwo );
	  		}
	  	//printf("11\n");
	  }
        }

        close(fd);
        gpio_fd_close(sysfs_fd);

        //return ret;
}

void ADCreader::quit()
{
	running = false;
	exit(0);
}
