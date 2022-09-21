/****************************************************************************
 * apps/examples/serialrx/ninaboot_main.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <nuttx/arch.h>
#include <nuttx/ioexpander/gpio.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * ninaboot_main
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  int fdusb, fdnina, fdgpio0, fdgpio1;
  int ret;
  char *buf, *buf2;
  char *devpathin, *devpathout;
  struct pollfd fds[2];
  struct termios tty;

  if (argc == 1)
    {
      devpathin = CONFIG_EXAMPLES_NINABOOT_DEVPATH_IN;
      devpathout = CONFIG_EXAMPLES_NINABOOT_DEVPATH_OUT;
    }
  else if (argc == 2)
    {
      devpathin = argv[1];
      devpathout = CONFIG_EXAMPLES_NINABOOT_DEVPATH_OUT;
    }
  else if (argc == 3)
    {
          devpathin = argv[1];
          devpathout = argv[2];
    }
  else
    {
      fprintf(stderr, "Usage: %s [devpathin] [devpathout]\n", argv[0]);
      goto errout;
    }

  buf = (char *)malloc(CONFIG_EXAMPLES_NINABOOT_BUFSIZE);
  memset(buf, 0, CONFIG_EXAMPLES_NINABOOT_BUFSIZE);
  if (buf == NULL)
    {
      fprintf(stderr, "ERROR: malloc failed: %d\n", errno);
      goto errout;
    }

  fdgpio0 = open("/dev/gpio0", O_RDWR);
  if (fdgpio0 < 0)
    {
      fprintf(stderr, "ERROR: open failed: %d\n", errno);
      goto errout_with_buf;
    }

  fdgpio1 = open("/dev/gpio1", O_RDWR);
  if (fdgpio1 < 0)
    {
      fprintf(stderr, "ERROR: open failed: %d\n", errno);
      goto errout_with_buf;
    }

  ret = ioctl(fdgpio1, GPIOC_WRITE, (unsigned long)0);
  ret = ioctl(fdgpio0, GPIOC_WRITE, (unsigned long)0);
  up_udelay(1000);
  ret = ioctl(fdgpio0, GPIOC_WRITE, (unsigned long)1);
  up_udelay(1000000);
  ret = ioctl(fdgpio1, GPIOC_WRITE, (unsigned long)1);

  fdusb = open(devpathin, O_RDWR|O_NONBLOCK);
  if (fdusb < 0)
    {
      fprintf(stderr, "ERROR: open failed: %d\n", errno);
      goto errout_with_buf;
    }

  ret = tcgetattr(fdusb, &tty);
  if (ret < 0)
    {
      printf("ERROR: Failed to get termios: %s\n", strerror(errno));
      goto errout_with_buf;
    }

  tty.c_oflag &= ~OPOST;

  ret = tcsetattr(fdusb, TCSANOW, &tty);
  if (ret < 0)
    {
      printf("ERROR: Failed to set termios: %s\n", strerror(errno));
      goto errout_with_buf;
    }

  fds[0].fd = fdusb;
  fds[0].events = POLLIN;

  fdnina = open(devpathout, O_RDWR|O_NONBLOCK);
  if (fdnina < 0)
    {
      fprintf(stderr, "ERROR: open failed: %d\n", errno);
      goto errout_with_buf;
    }

  fds[1].fd = fdnina;
  fds[1].events = POLLIN;

  printf("Bridging %s with %s\n", devpathin, devpathout);
  fflush(stdout);

  while (1)
    {
      up_udelay(1000);
      poll(fds, 2, -1);
      if (fds[0].revents & POLLIN)
        {
          ret = read(fdusb, buf, CONFIG_EXAMPLES_NINABOOT_BUFSIZE);
          if (ret > 0)
            {
              write(fdnina, buf, ret);
            }
          else if (ret < 0)
            {
              printf("read USB failed: %d\n", errno);
              fflush(stdout);
              break;
            } 
        }

      if (fds[1].revents & POLLIN)
        {
          ret = read(fdnina, buf, CONFIG_EXAMPLES_NINABOOT_BUFSIZE);
          if (ret > 0)
            {
              write(fdusb, buf, ret); 
            }
          else if (ret < 0)
            {
              printf("read NINA failed: %d\n", errno);
              fflush(stdout);
              break;
            }  
        }
    }

  printf("Terminated\n");
  fflush(stdout);

  up_udelay(1000000);
  close(fdusb);
  close(fdnina);
  close(fdgpio0);
  close(fdgpio1);

  free(buf);
  return EXIT_SUCCESS;

errout_with_buf:
  free(buf);

errout:
  fflush(stderr);
  return EXIT_FAILURE;
}
